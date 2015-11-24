/* Tetris-like falling blocks game for JY-MCY 3208 "Lattice Clock"
 *
 * Copyright (C) Pete Hollobon 2015
 */

#include <avr/eeprom.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
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

#define M_KEY_DOWN 8
#define M_KEY_UP 9
#define M_KEY_REPEAT 10

#define key_down(n) ((PIND & (1 << ((n) + 5))) == 0)

uint32_t EEMEM high_score_address = 0;
uint8_t EEMEM high_score_name_address[3] = "---";
volatile uint16_t clock_count = 0;

void set_up_keys(void)
{
    /* Set high 3 bits of port D as input */
    DDRD &= 0b00011111;

    /* Turn on pull-up resistors on high 3 bits */
    PORTD |= 0b11100000;
}

void set_up_timer(void)
{
    cli();                      /* disable interrupts */

    ICR1 = F_CPU / 1000;        /* input capture register 1 - interrupt frequency 1000Hz */

    TCCR1A = 0;                 /* zero output compare stuff and the low two WGM bits */

    // timer counter control register 1 B: Mode 12, CTC with ICR1 as TOP, no prescaling
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

    // Set interrupt on compare match
    TIMSK = (1 << TICIE1);

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

#define MAX_TIMERS 1

#if MAX_TIMERS > 32
#error MAX_TIMERS must be <= 32
typedef uint32_t timer_bitfield;
#elif MAX_TIMERS > 16
typedef uint32_t timer_bitfield;
#elif MAX_TIMERS > 8
typedef uint16_t timer_bitfield;
#else
typedef uint8_t timer_bitfield;
#endif

typedef struct {
    uint16_t ms;
    uint16_t end;
} timer_t;
timer_t timers[MAX_TIMERS];
timer_bitfield timers_active = 0;
timer_bitfield timers_recur = 0;

void set_timer(uint16_t ms, uint8_t n, bool recur)
{
    timers_active |= 1 << n;
    if (recur)
        timers_recur |= 1 << n;
    timers[n].ms = ms;
    timers[n].end = clock_count + ms;
}

void stop_timer(uint8_t n)
{
    timers_active &= ~(1 << n);
}

/* Interrupt handler for timer1. Polls keys and pushes events onto message queue. */
ISR (TIMER1_CAPT_vect)
{
    static bool key_seen[3] = {false, false, false};
    static bool key_repeating[3] = {false, false, false};
    static uint16_t key_clock[3] = {0, 0, 0};
    static uint16_t repeat_delay[3] = {300, 50, 300};

    for (int key = 0; key < 3; key++) {
        if (key_down(key)) {
            if (!key_seen[key]) {
                key_clock[key] = clock_count;
                key_seen[key] = true;
                mq_put(msg_create(M_KEY_DOWN, key));
            }
            else if (clock_count - key_clock[key] > (key_repeating[key] ? repeat_delay[key] : 300)) {
                key_clock[key] = clock_count;
                mq_put(msg_create(M_KEY_REPEAT, key));
                key_repeating[key] = true;
            }
        }
        else if (key_seen[key]) {
            key_seen[key] = false;
            mq_put(msg_create(M_KEY_UP, key));
            key_repeating[key] = false;
        }
    }

    for (int timer = 0; timer < MAX_TIMERS; timer++) {
        if (timers_active & (1 << timer) && clock_count > timers[timer].end) {
            if (!(timers_recur & (1 << timer)))
                stop_timer(timer);
            else
                timers[timer].end = clock_count + timers[timer].ms;
            mq_put(msg_create(M_TIMER, timer));
        }
    }

    clock_count++;
}

/* Copy the source board to the destination, overlaying shape at the row specified */
void overlay_shape(uint8_t src[32], uint8_t dest[32], uint8_t shape[4], uint8_t shape_top)
{
    int n;
    for (n = 0; n < 32; n++) {
        if (n >= shape_top && n - shape_top < 4)
            dest[n] = src[n] | shape[n - shape_top];
        else
            dest[n] = src[n];
    }
}

/* Offset a shape to the right by a positive number of pixels */
void offset_shape(uint8_t shape[4], uint8_t n)
{
    int row;
    for (row = 0; row < 4; row++)
        shape[row] >>= n;
}

/* Test if overlaying the shape on the board at the specified line would
 * result in a collision
 */
bool test_collision(uint8_t board[32], uint8_t shape[4], uint8_t line)
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
    int flash_count;
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
int collapse_full_rows(uint8_t src[32], uint8_t dest[32])
{
    int src_index, dest_index;

    dest_index = 31;
    for (src_index = 31; src_index >= 0; src_index--) {
        if (src[src_index] != 0xff)
            dest[dest_index--] = src[src_index];
    }
    src_index = dest_index + 1;
    while (dest_index >= 0)
        dest[dest_index--] = 0;

    return src_index;
}

/* Get the "width" of a shape: the column number of the rightmost set pixel */
uint8_t get_shape_width(uint8_t shape[4])
{
    uint8_t min = 8;
    uint8_t row, bit;
    for (row = 0; row < 4; row++) {
        for (bit = 0; bit < 8; bit++) {
            if (shape[row] & (1 << bit)) {
                if (bit < min)
                    min = bit;
                break;
            }
        }
    }
    return 8 - min;
}

bool render_number(uint32_t number, byte board[32])
{
    ldiv_t q;
    character c;
    int pos = 0;
    uint32_t n = number;

    // calculate position of least significant digit
    do {
        q = ldiv(n, 10);
        n = q.quot;
        c = numbers[q.rem];
        pos += c.columns + 1;
    } while (n);
    pos -= 1;

    if (pos > 31)
        // overflows board
        return false;

    // render
    do {
        q = ldiv(number, 10);
        number = q.quot;
        c = numbers[q.rem];
        for (int i = c.columns - 1; i >= 0; i--) {
            board[pos--] = c.bitmap[i];
        }
        pos--;
    } while (number);

    return true;
}

bool render_string(char* string, byte board[32])
{
    int pos = 1;
    character *c;
    while (*string) {
        if (*string == ' ')
            pos += 3;
        else {
            c = &letters[*string - 65];
            for (int i = 0; i < c->columns; i++) {
                board[pos++] = c->bitmap[i];
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

void read_name(char* name)
{
    uint8_t message;
    int position = 0;
    memset(name, 0, 4);
    name[0] = 'A';

    memset(leds, 0, 32);
    render_string(name, leds);
    HTsendscreen();

    while (position < 3) {
        if (mq_get(&message)) {
            if (msg_get_event(message) == M_KEY_DOWN || msg_get_event(message) == M_KEY_REPEAT) {
                switch (msg_get_param(message)) {
                case 0:         /* left */
                    if (name[position] > 'A')
                        name[position]--;
                    break;
                case 1:         /* middle */
                    position++;
                    if (position < 3)
                        name[position] = 'A';
                    break;
                case 2:         /* right */
                    if (name[position] < 'Z')
                        name[position]++;
                    break;
                }
                memset(leds, 0, 32);
                render_string(name, leds);
                HTsendscreen();
            }
        }
    }
    memset(leds, 0, 32);
    HTsendscreen();
}

#define MOVE_LEFT 1
#define MOVE_RIGHT 2
#define ROTATE 3
#define DROP 4

#define MIN_DROP_INTERVAL 150
#define DROP_INCREMENT 30

uint8_t row_scores[] = {0, 1, 4, 8, 16};

int main(void)
{
    uint8_t board[32];
    uint8_t shape_top;
    uint8_t shape_offset, proposed_shape_offset;
    uint8_t shape_rotation, proposed_shape_rotation;
    uint8_t shape_selection;
    uint8_t shape_width;
    uint8_t shape[4], proposed_shape[4];
    unsigned int last_block_move_clock;
    unsigned int drop_interval = 600;
    unsigned int last_drop_reduction_clock;
    bool update_shape;
    uint8_t message;
    uint8_t action;
    uint8_t key1_autorepeat = false;
    uint32_t score = 0, high_score = 0;
    bool new_high_score = true;
    char high_score_name[4];

    HTpinsetup();
    HTsetup();
    set_up_keys();
    set_up_timer();
    set_up_rand();
    HTbrightness(1);
    memset(timers, 0, MAX_TIMERS);

    score = eeprom_read_dword(&high_score_address);
    eeprom_read_block(high_score_name, &high_score_name_address, 3);
    high_score_name[3] = 0;

    while (1) {
        // Flash score / high score until a button is pressed
        action = 0;
        memset(leds, 0, 32);
        render_number(score, leds);
        memcpy(board, leds, 32);
        set_timer(900, 0, true);
        while (1) {
            if (mq_get(&message)) {
                if (msg_get_event(message) == M_KEY_DOWN) {
                    memset(leds, 0, 32);
                    memset(board, 0, 32);
                    HTsendscreen();
                    break;
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
                        memcpy(leds, board, 32);
                        action = 0;
                    }

                    HTsendscreen();
                }
            }
        }
        stop_timer(0);

        score = 0;
        new_high_score = false;
        shape_top = 0;
        shape_offset = 3;
        shape_rotation = rand() % 4;
        shape_selection = rand() % 7;
        shape_width = get_shape_width(shapes[shape_selection][shape_rotation]);
        memcpy(shape, shapes[shape_selection][shape_rotation], 4);
        offset_shape(shape, shape_offset);

        last_block_move_clock = clock_count;
        last_drop_reduction_clock = clock_count;

        // Main game loop
        while (1) {
            if (drop_interval > MIN_DROP_INTERVAL && clock_count - last_drop_reduction_clock > 15000) {
                drop_interval -= DROP_INCREMENT;
                last_drop_reduction_clock = clock_count;
            }

            action = 0;
            if (mq_get(&message)) {
                if (msg_get_event(message) == M_KEY_DOWN || msg_get_event(message) == M_KEY_REPEAT) {
                    switch (msg_get_param(message)) {
                    case 0:
                        action = MOVE_LEFT;
                        break;
                    case 2:
                        action = MOVE_RIGHT;
                    }
                }
                if (msg_get_param(message) == 1) {
                    if (msg_get_event(message) == M_KEY_REPEAT) {
                        key1_autorepeat = true;
                        action = DROP;
                    }

                    else if (msg_get_event(message) == M_KEY_UP) {
                        if (key1_autorepeat)
                            key1_autorepeat = false;
                        else
                            action = ROTATE;
                    }
                }
            }

            if (action == DROP || (clock_count - last_block_move_clock) > drop_interval) {
                last_block_move_clock = clock_count;
                if (test_collision(board, shape, shape_top + 1)) {
                    if (shape_top == 0)
                        // Game over
                        break;

                    flash_full_rows();
                    score += row_scores[collapse_full_rows(leds, board)];

                    shape_top = 0;
                    shape_offset = 3;
                    shape_rotation = rand() % 4;
                    shape_selection = rand() % 7;
                    shape_width = get_shape_width(shapes[shape_selection][shape_rotation]);
                    memcpy(shape, shapes[shape_selection][shape_rotation], 4);
                    offset_shape(shape, shape_offset);
                }
                else {
                    shape_top++;
                }

                overlay_shape(board, leds, shape, shape_top);
                HTsendscreen();
            }

            update_shape = false;
            switch (action) {
            case MOVE_LEFT:
                if (shape_offset > 0) {
                    proposed_shape_offset = shape_offset - 1;
                    if (proposed_shape_offset < 0)
                        proposed_shape_offset = 0;
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
                proposed_shape_rotation = (shape_rotation + 1) % 4;
                proposed_shape_offset = shape_offset;
                if (shape_offset + get_shape_width(shapes[shape_selection][proposed_shape_rotation]) < 9)
                    update_shape = true;
                break;
            }

            if (update_shape) {
                memcpy(proposed_shape, shapes[shape_selection][proposed_shape_rotation], 4);
                offset_shape(proposed_shape, proposed_shape_offset);

                if (!test_collision(board, proposed_shape, shape_top)) {
                    shape_offset = proposed_shape_offset;
                    shape_rotation = proposed_shape_rotation;
                    memcpy(shape, proposed_shape, 4);
                    shape_width = get_shape_width(shapes[shape_selection][shape_rotation]);
                }

                overlay_shape(board, leds, shape, shape_top);
                HTsendscreen();
            }
        }

        high_score = eeprom_read_dword(&high_score_address);
        if (score > high_score) {
            read_name(high_score_name);
            eeprom_update_block(high_score_name, &high_score_name_address, 3);
            eeprom_write_dword(&high_score_address, score);
            new_high_score = true;
        }
    }
}
