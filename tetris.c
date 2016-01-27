/* Tetris-like falling blocks game for JY-MCY 3208 "Lattice Clock"
 *
 * Copyright (C) Pete Hollobon 2015
 */

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ht1632c.h"
#include "mq.h"
#include "tetris.h"
#include "numbers.h"
#include "letters.h"
#include "music.h"
#include "timers.h"

#define M_KEY_DOWN 8
#define M_KEY_UP 9
#define M_KEY_REPEAT 10

#define DEBOUNCE_TIME 10

#define MOVE_LEFT 1
#define MOVE_RIGHT 2
#define ROTATE 3
#define DROP 4

// Difficulty configuration
#define INITIAL_DROP_INTERVAL 600
#define MIN_DROP_INTERVAL 150
#define DROP_INTERVAL_INCREMENT 30
#define INTERVAL_DECREASE_LINES 10

const uint8_t row_scores[] PROGMEM = {0, 1, 4, 8, 16};

#define key_down(n) ((PIND & (1 << ((n) + 5))) == 0)

uint32_t EEMEM high_score_address = 0;
uint8_t EEMEM high_score_name_address[3] = "   ";

void set_up_keys(void)
{
    /* Set high 3 bits of port D as input */
    DDRD &= 0b00011111;

    /* Turn on pull-up resistors on high 3 bits */
    PORTD |= 0b11100000;

    DDRC |= 1 << 5;
}

void set_up_timers(void)
{
    cli();                      /* disable interrupts */

    ICR1 = F_CPU / 1000;        /* input capture register 1 - interrupt frequency 1000Hz */

    TCCR1A = 0;                 /* zero output compare stuff and the low two WGM bits */

    // timer counter control register 1 B: Mode 12, CTC with ICR1 as TOP, no prescaling
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

    // Set interrupt on compare match
    TIMSK = (1 << TICIE1);

    // timer 2: CTC, OCR2 as TOP, clock / 64
    TCCR2 |= _BV(WGM21) | _BV(CS22);

    sei();
}

void set_up_rand(void)
{
    static uint16_t EEMEM rand_seed;
    uint16_t seed_value;

    seed_value = eeprom_read_word(&rand_seed);
    srand(seed_value);
    eeprom_write_word(&rand_seed, seed_value + 1);
}

/* Interrupt handler for timer1. Polls keys and pushes events onto message queue. */
ISR (TIMER1_CAPT_vect, ISR_NOBLOCK)
{
    typedef enum {down, up} key_state;
    static key_state last_state[3] = {up, up, up};
    static bool steady_state[3] = {true, true, true};
    static bool key_repeating[3] = {false, false, false};
    static uint16_t state_change_clock[3] = {0, 0, 0};
    static uint16_t repeat_initial_delay[3] = {300, 300, 300};
    static uint16_t repeat_subsequent_delay[3] = {200, 50, 200};

    for (int key = 0; key < 3; key++) {
        if (key_down(key)) {
            if (last_state[key] == up) {
                state_change_clock[key] = clock_count;
                last_state[key] = down;
                steady_state[key] = false;
            }
            else if (clock_count - state_change_clock[key] > (key_repeating[key] ? repeat_subsequent_delay[key] : repeat_initial_delay[key])) {
                state_change_clock[key] = clock_count;
                mq_put(msg_create(M_KEY_REPEAT, key));
                key_repeating[key] = true;
            }
            else if (!steady_state[key] && clock_count - state_change_clock[key] > DEBOUNCE_TIME) {
                mq_put(msg_create(M_KEY_DOWN, key));
                steady_state[key] = true;
            }
        }
        else {
            if (last_state[key] == down) {
                last_state[key] = up;
                steady_state[key] = false;
                state_change_clock[key] = clock_count;
            }
            else if (!steady_state[key] && clock_count - state_change_clock[key] > DEBOUNCE_TIME) {
                mq_put(msg_create(M_KEY_UP, key));
                steady_state[key] = true;
                key_repeating[key] = false;
            }
        }
    }

    handle_timers();
}

/* Interrupt handler for timer2. Bitbangs a square wave for audio */
ISR (TIMER2_COMP_vect)
{
    PORTC ^= 1 << 5;
}

/* Overlay shape at the row specified using XOR */
void overlay_shape(uint8_t board[32], uint8_t shape[4], uint8_t shape_top)
{
    uint8_t n;
    for (n = 0; n < 4; n++)
        board[shape_top + n] ^= shape[n];
}

/* Offset a shape to the right by a positive number of pixels */
void offset_shape(uint8_t shape[4], uint8_t n)
{
    uint8_t row;
    for (row = 0; row < 4; row++)
        *shape++ >>= n;
}

/* Test if overlaying the shape on the board at the specified line would
 * result in a collision
 */
bool test_collision(const uint8_t board[32], const uint8_t shape[4], uint8_t line)
{
    uint8_t shape_length = 0;
    uint8_t n;

    while (shape_length < 4 && shape[shape_length])
        shape_length++;

    if (line + shape_length > 32)
        return true;

    for (n = 0; n < shape_length; n++) {
        if (board[line + n] & shape[n])
            return true;
    }
    return false;
}

void flash_full_rows(void)
{
    int row;
    uint8_t flash_count;
    uint32_t flash_rows = 0;

    for (row = 0; row < 32; row++) {
        if (leds[row] == 0xff)
            flash_rows |= 1UL << row;
    }

    if (flash_rows) {
        for (flash_count = 0; flash_count < 4; flash_count++) {
            for (row = 0; row < 32; row++) {
                if (flash_rows & (1UL << row))
                    leds[row] ^= 0xFF;
            }
            HTsendscreen();
            _delay_ms(100);
        }
    }
}

/* Copy the source board to the destination, omitting any complete rows
 * Returns the number of completed rows that were collapsed
 */
uint8_t collapse_full_rows(uint8_t dest[32])
{
    int8_t src_index = 31, dest_index = 31;
    for (; src_index >= 0; src_index--) {
        if (dest[src_index] != 0xff)
            dest[dest_index--] = dest[src_index];
    }
    src_index = dest_index + 1;
    while (dest_index >= 0)
        dest[dest_index--] = 0;

    return src_index;
}

/* Get the "width" of a shape: the column number of the rightmost set pixel */
uint8_t get_shape_width(const uint8_t shape[4])
{
    uint8_t all, bit = 8;

    for (all = shape[0] | shape[1] | shape[2] | shape[3]; all && !(all & 1); all >>= 1)
        bit--;
    return bit;
}

bool render_number(uint32_t number, byte board[32])
{
    ldiv_t q;
    const character *c;
    uint8_t pos = 0;
    int8_t i;
    uint32_t n = number;

    // calculate position of least significant digit
    do {
        q = ldiv(n, 10);
        n = q.quot;
        c = &numbers[q.rem];
        pos += c->columns + 1;
    } while (n);
    pos--;

    if (pos > 31)
        // overflows board
        return false;

    // render
    do {
        q = ldiv(number, 10);
        number = q.quot;
        c = &numbers[q.rem];
        for (i = c->columns - 1; i >= 0; i--) {
            board[pos--] = pgm_read_byte(&(c->bitmap[i]));
        }
        pos--;
    } while (number);

    return true;
}

bool render_string(const char* string, byte board[32])
{
    uint8_t pos = 1, i;
    const character *c;
    while (*string) {
        if (*string == ' ')
            pos += 3;
        else if (*string < 'A' || *string > 'Z')
            return false;
        else {
            c = &letters[*string - 65];
            for (i = 0; i < c->columns; i++) {
                board[pos++] = pgm_read_byte(&(c->bitmap[i]));
                if (pos > 31)
                    return false;
            }
            pos++;
        }
        if (pos > 31)
            return false;
        string++;
    }

    return true;
}

void read_string(char* name, uint8_t length)
{
    message_t message;
    uint8_t position = 0;
    char current_char = 'A';
    bool show_current_char = true;
    memset(name, 0, length + 1);
    name[0] = 'A';

    memset(leds, 0, 32);
    render_string(name, leds);
    HTsendscreen();

    set_timer(250, 0, true);
    while (position < length) {
        if (mq_get(&message)) {
            if (msg_get_event(message) == M_TIMER && msg_get_param(message) == 0) {
                if (show_current_char) {
                    name[position] = current_char;
                }
                else {
                    name[position] = 0;
                }

                show_current_char ^= true;
                memset(leds, 0, 32);
                render_string(name, leds);
                HTsendscreen();
            }
            else if (msg_get_event(message) == M_KEY_DOWN || msg_get_event(message) == M_KEY_REPEAT) {
                switch (msg_get_param(message)) {
                case 0:         /* left */
                    if (current_char > 'A')
                        current_char--;
                    break;
                case 2:         /* right */
                    if (current_char < 'Z')
                        current_char++;
                    break;
                }
                name[position] = current_char;
                show_current_char = false;
                set_timer(250, 0, true);

                memset(leds, 0, 32);
                render_string(name, leds);
                HTsendscreen();
            }
            if (msg_get_event(message) == M_KEY_DOWN && msg_get_param(message) == 1) {
                name[position] = current_char;
                position++;
                if (position < length) {
                    current_char = 'A';
                    name[position] = current_char;
                    show_current_char = false;
                    set_timer(250, 0, true);

                    memset(leds, 0, 32);
                    render_string(name, leds);
                    HTsendscreen();
                }
            }
        }
    }
    stop_timer(0);
    memset(leds, 0, 32);
    HTsendscreen();
}

int main(void)
{
    uint8_t shape_top;
    uint8_t shape_offset, proposed_shape_offset;
    uint8_t shape_rotation, proposed_shape_rotation;
    uint8_t shape_selection;
    uint8_t shape_width;
    uint8_t shape[4], proposed_shape[4];
    bool update_shape;
    message_t message;
    uint8_t action;
    uint8_t key1_autorepeat = false;
    uint32_t score = 0, high_score = 0;
    bool new_high_score = true;
    char high_score_name[4];
    uint8_t rows_cleared;
    uint8_t drop_interval_line_count = 0;
    uint16_t drop_interval = INITIAL_DROP_INTERVAL;

    HTpinsetup();
    HTsetup();
    set_up_keys();
    set_up_timers();
    set_up_rand();
    init_timers();
    HTbrightness(1);

    score = eeprom_read_dword(&high_score_address);
    eeprom_read_block(high_score_name, &high_score_name_address, 3);
    high_score_name[3] = 0;

    while (1) {
        // Flash score / high score until a button is pressed
        action = 0;
        memset(leds, 0, 32);
        set_timer(900, 0, true);
        start_music();
        while (1) {
            if (mq_get(&message)) {
                if (msg_get_event(message) == M_KEY_DOWN) {
                    break;
                }
                else if (msg_get_event(message) == M_TIMER && msg_get_param(message) == 1) {
                    handle_music();
                }
                else if (msg_get_event(message) == M_TIMER && msg_get_param(message) == 0) {
                    if (action == 0) { /* show "score" / "hi score" */
                        memset(leds, 0, 32);
                        if (new_high_score) {
                            render_string("HI SCORE", leds);
                            action = 1;
                        }
                        else {
                            render_string("SCORE", leds);
                            action = 2;
                        }
                    }
                    else if (action == 1) { /* show high score name */
                        memset(leds, 0, 32);
                        render_string(high_score_name, leds);
                        action = 2;
                    }
                    else if (action == 2) { /* show score */
                        memset(leds, 0, 32);
                        render_number(score, leds);
                        action = 0;
                    }

                    HTsendscreen();
                }
            }
        }
        stop_timer(0);
        stop_music();

        score = 0;
        new_high_score = false;
        shape_top = 0;
        shape_offset = 3;
        shape_rotation = rand() & 3;
        shape_selection = rand() % 7;
        shape_width = get_shape_width(shapes[shape_selection][shape_rotation]);
        memcpy(shape, shapes[shape_selection][shape_rotation], 4);
        offset_shape(shape, shape_offset);
        memset(leds, 0, 32);
        overlay_shape(leds, shape, shape_top);

        set_timer(drop_interval, 0, true);

        // Main game loop
        while (1) {
            action = 0;
            if (mq_get(&message)) {
                if (msg_get_event(message) == M_TIMER) {
                    switch (msg_get_param(message)) {
                    case 0:
                        action = DROP;
                        break;
                    case 1:
                        handle_music();
                        break;
                    }
                }
                else if (msg_get_event(message) == M_KEY_DOWN || msg_get_event(message) == M_KEY_REPEAT) {
                    switch (msg_get_param(message)) {
                    case 0:
                        action = MOVE_LEFT;
                        break;
                    case 2:
                        action = MOVE_RIGHT;
                        break;
                    }
                }
                if (msg_get_event(message) == M_KEY_REPEAT && msg_get_param(message) == 1) {
                    key1_autorepeat = true;
                    action = DROP;
                }
                else if (msg_get_event(message) == M_KEY_UP && msg_get_param(message) == 1) {
                    if (key1_autorepeat)
                        key1_autorepeat = false;
                    else
                        action = ROTATE;
                }
            }

            if (action == DROP) {
                // erase previous shape
                overlay_shape(leds, shape, shape_top);
                
                if (test_collision(leds, shape, shape_top + 1)) {
                    if (shape_top == 0)
                        // Game over
                        break;

                    // draw back shape, as it has landed now
                    overlay_shape(leds, shape, shape_top);

                    flash_full_rows();
                    rows_cleared = collapse_full_rows(leds);
                    score += pgm_read_byte(&(row_scores[rows_cleared]));
                    drop_interval_line_count += rows_cleared;
                    if (drop_interval_line_count >= INTERVAL_DECREASE_LINES && drop_interval > MIN_DROP_INTERVAL) {
                        drop_interval -= DROP_INTERVAL_INCREMENT;
                        drop_interval_line_count -= INTERVAL_DECREASE_LINES;
                        set_timer(drop_interval, 0, true);
                    }
                    shape_top = 0;
                    shape_offset = 3;
                    shape_rotation = rand() & 3;
                    shape_selection = rand() % 7;
                    shape_width = get_shape_width(shapes[shape_selection][shape_rotation]);
                    memcpy(shape, shapes[shape_selection][shape_rotation], 4);
                    offset_shape(shape, shape_offset);
                }
                else {
                    shape_top++;
                }

                // overlay new falling shape
                overlay_shape(leds, shape, shape_top);
                HTsendscreen();
            }

            update_shape = false;
            switch (action) {
            case MOVE_LEFT:
                if (shape_offset > 0) {
                    proposed_shape_offset = shape_offset - 1;
                    proposed_shape_rotation = shape_rotation;
                    update_shape = true;
                }
                break;

            case MOVE_RIGHT:
                proposed_shape_offset = shape_offset + 1;
                if (proposed_shape_offset + shape_width > 8)
                    proposed_shape_offset = 8 - shape_width;
                proposed_shape_rotation = shape_rotation;
                update_shape = true;
                break;

            case ROTATE:
                proposed_shape_rotation = (shape_rotation + 1) & 3;
                proposed_shape_offset = shape_offset;
                if (shape_offset + get_shape_width(shapes[shape_selection][proposed_shape_rotation]) < 9)
                    update_shape = true;
                break;
            }

            if (update_shape) {
                memcpy(proposed_shape, shapes[shape_selection][proposed_shape_rotation], 4);
                offset_shape(proposed_shape, proposed_shape_offset);

                // erase shape in previous position
                overlay_shape(leds, shape, shape_top);

                if (!test_collision(leds, proposed_shape, shape_top)) {
                    shape_offset = proposed_shape_offset;
                    shape_rotation = proposed_shape_rotation;
                    memcpy(shape, proposed_shape, 4);
                    shape_width = get_shape_width(shapes[shape_selection][shape_rotation]);
                }

                // overlay shape in new position
                overlay_shape(leds, shape, shape_top);
                HTsendscreen();
            }
        }

        stop_timer(0);

        high_score = eeprom_read_dword(&high_score_address);
        if (score > high_score) {
            read_string(high_score_name, 3);
            eeprom_update_block(high_score_name, &high_score_name_address, 3);
            eeprom_write_dword(&high_score_address, score);
            new_high_score = true;
        }
    }
}
