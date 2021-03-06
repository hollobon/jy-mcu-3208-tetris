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
#include "timers.h"
#include "io.h"
#include "music.h"
#include "lookup.h"

#define M_FADE_COMPLETE 11

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

uint32_t EEMEM high_score_address = 0;
uint8_t EEMEM high_score_name_address[3] = "   ";

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

/* Seed random number generator with a value from EEPROM, and update with a new seed
 *
 * If this wasn't done, the sequence of shapes would be identical after every reset.
 */
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
    handle_keys();
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

uint8_t collapse_full_rows(void)
{
    int row;
    int8_t time_index;
    uint32_t full_rows = 0;
    message_t message;

    for (row = 0; row < 32; row++) {
        if (leds[row] == 0xff)
            full_rows |= 1UL << row;
    }

    if (full_rows) {
        time_index = 15;
        goto start;
        while (time_index > 0) {
            if (mq_get(&message)) {
                if (msg_get_event(message) == M_TIMER && msg_get_param(message) == 0) {
                start:
                    for (row = 0; row < 32; row++) {
                        if (full_rows & (1UL << row)) {
                            if (row & 1)
                                leds[row] >>= 1;
                            else
                                leds[row] <<= 1;
                        }
                    }
                    HTsendscreen();
                    set_timer(cos_s16_r1_15(time_index) << 3, 0, false);
                    time_index -= 2;
                }
            }
        }

        int8_t src_index = 31, dest_index = 31;
        uint8_t full_row_count;
        uint32_t row_mask = (1UL << 31);
        for (; src_index >= 0; src_index--, row_mask >>= 1) {
            if (!(full_rows & row_mask))
                leds[dest_index--] = leds[src_index];
        }
        full_row_count = dest_index + 1;
        while (dest_index >= 0)
            leds[dest_index--] = 0;

        return full_row_count;
    }
    return 0;
}

/* Get the "width" of a shape: the column number of the rightmost set pixel */
uint8_t get_shape_width(const uint8_t shape[4])
{
    uint8_t all, bit = 8;

    for (all = shape[0] | shape[1] | shape[2] | shape[3]; all && !(all & 1); all >>= 1)
        bit--;
    return bit;
}

static uint8_t fade_value = 0;
static bool do_fade_in;

void handle_fade(void)
{
    HTbrightness(cos_s16_r1_15(fade_value));
    if ((do_fade_in && fade_value == 15)
        || (!do_fade_in && fade_value == 0)) {
        stop_timer(2);
        mq_put(msg_create(M_FADE_COMPLETE, 0));
    }
    else {
        if (do_fade_in)
            fade_value++;
        else
            fade_value--;
    }
}

void fade_in(uint8_t speed)
{
    fade_value = 0;
    do_fade_in = true;
    set_timer(speed, 2, true);
}

void fade_out(uint8_t speed)
{
    fade_value = 15;
    do_fade_in = false;
    set_timer(speed, 2, true);
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
    uint8_t action, next_action = 0;
    uint8_t key1_autorepeat = false;
    uint32_t score = 0, high_score = 0;
    bool new_high_score = true;
    char high_score_name[4];
    uint8_t rows_cleared;
    uint8_t drop_interval_line_count = 0;
    uint16_t drop_interval = INITIAL_DROP_INTERVAL;

    HTpinsetup();
    HTsetup();
    init_keys();
    set_up_timers();
    set_up_rand();
    init_timers();

    score = eeprom_read_dword(&high_score_address);
    eeprom_read_block(high_score_name, &high_score_name_address, 3);
    high_score_name[3] = 0;

    #define A_SHOW_SCORE_TYPE 0
    #define A_SHOW_HIGH_SCORE_NAME 1
    #define A_SHOW_SCORE 2
    #define A_FADE_IN 10
    #define A_FADE_OUT 11

    while (1) {
        // Flash score / high score until a button is pressed
        action = 0;
        memset(leds, 0, 32);
        set_timer(900, 0, true);
        start_music();
        while (1) {
            if (mq_get(&message)) {
                if (msg_get_event(message) == M_KEY_DOWN)
                    break;
                else switch (msg_get_event(message)) {
                    case M_TIMER:
                        switch (msg_get_param(message)) {
                        case 0:
                            switch (action) {
                            case A_SHOW_SCORE_TYPE:
                                memset(leds, 0, 32);
                                if (new_high_score) {
                                    render_string("HI SCORE", leds);
                                    next_action = A_SHOW_HIGH_SCORE_NAME;
                                }
                                else {
                                    render_string("SCORE", leds);
                                    next_action = A_SHOW_SCORE;
                                }
                                action = A_FADE_IN;
                                fade_in(20);
                                break;
                            case A_SHOW_HIGH_SCORE_NAME:
                                memset(leds, 0, 32);
                                render_string(high_score_name, leds);
                                next_action = A_SHOW_SCORE;
                                action = A_FADE_IN;
                                fade_in(20);
                                break;
                            case A_SHOW_SCORE:
                                memset(leds, 0, 32);
                                render_number(score, leds);
                                next_action = A_SHOW_SCORE_TYPE;
                                action = A_FADE_IN;
                                fade_in(20);
                                break;
                            case A_FADE_OUT:
                                action = next_action;
                                fade_out(20);
                                break;
                            }

                            HTsendscreen();
                            break;
                        case 1:
                            handle_music();
                            break;
                        case 2:
                            handle_fade();
                        }
                        break;
                    case M_FADE_COMPLETE:
                        if (action == A_FADE_IN)
                            action = A_FADE_OUT;
                        else {
                            set_timer(900, 0, true);
                            mq_put(msg_create(M_TIMER, 0));
                        }
                        break;
                    }
            }
        }
        stop_timer(0);
        stop_music();
        HTbrightness(1);

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
                    case KEY_LEFT:
                        action = MOVE_LEFT;
                        break;
                    case KEY_RIGHT:
                        action = MOVE_RIGHT;
                        break;
                    }
                }
                if (msg_get_event(message) == M_KEY_REPEAT && msg_get_param(message) == KEY_MIDDLE) {
                    key1_autorepeat = true;
                    action = DROP;
                }
                else if (msg_get_event(message) == M_KEY_UP && msg_get_param(message) == KEY_MIDDLE) {
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

                    rows_cleared = collapse_full_rows();
                    score += pgm_read_byte(&(row_scores[rows_cleared]));
                    drop_interval_line_count += rows_cleared;
                    if (drop_interval_line_count >= INTERVAL_DECREASE_LINES && drop_interval > MIN_DROP_INTERVAL) {
                        drop_interval -= DROP_INTERVAL_INCREMENT;
                        drop_interval_line_count -= INTERVAL_DECREASE_LINES;
                    }
                    set_timer(drop_interval, 0, true);
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
            read_string(high_score_name, 3, 0);
            eeprom_update_block(high_score_name, &high_score_name_address, 3);
            eeprom_write_dword(&high_score_address, score);
            new_high_score = true;
        }
    }
}
