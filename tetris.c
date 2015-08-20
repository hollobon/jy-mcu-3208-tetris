/* Tetris-like falling blocks game for JY-MCY 3208 "Lattice Clock"
 *
 * Copyright (C) Pete Hollobon 2015
 */

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ht1632c.h"
#include "tetris.h"

#define key1 ((PIND & (1 << 7)) == 0)
#define key2 ((PIND & (1 << 6)) == 0)
#define key3 ((PIND & (1 << 5)) == 0)

#define key_down(n) ((PIND & (1 << ((n) + 5))) == 0)

#define MQ_SIZE 20
uint8_t _mq[MQ_SIZE];
uint8_t _mq_front = 0;
uint8_t _mq_back = 0;

#define mq_empty (_mq_front == _mq_back)
#define mq_used ((_mq_front - _mq_back) % MQ_SIZE)
#define mq_space (MQ_SIZE - mq_used)

bool mq_put(uint8_t x)
{
    if (mq_space < 1)
        return false;

    _mq[_mq_front] = x;
    _mq_front = (_mq_front + 1) % MQ_SIZE;
    return true;
}

bool mq_get(uint8_t *value)
{
    if (mq_empty)
        return false;
    *value = _mq[_mq_back];
    _mq_back = (_mq_back + 1) % MQ_SIZE;
    return true;
}

#define M_KEY_DOWN 8
#define M_KEY_UP 9
#define M_KEY_REPEAT 10

#define msg_create(event, param) (param << 4 | (event & 0x0F))
#define msg_get_event(msg) (msg & 0x0F)
#define msg_get_param(msg) (msg >> 4)

volatile uint16_t clock_count = 0;
volatile uint16_t key1_time = 0;

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

    ICR1 = 7810;                /* input capture register 1 */

    TCCR1A = 0;                 /* zero output compare stuff and the low two WGM bits */

    // timer counter control register 1 B: Mode 12, CTC on ICR1, no prescaling
    TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);

    //Set interrupt on compare match
    TIMSK = (1 << TICIE1);

    sei();
}

ISR (TIMER1_CAPT_vect)
{
    static bool key_seen[3] = {false, false, false};
    static uint16_t key_clock[3] = {0, 0, 0};
    int key;

    for (key = 0; key < 3; key++) {
        if (key_down(key)) {
            if (!key_seen[key]) {
                key_clock[key] = clock_count;
                key_seen[key] = true;
                mq_put(msg_create(M_KEY_DOWN, key));
            }
            else if (clock_count - key_clock[key] > 300) {
                key_clock[key] = clock_count;
                mq_put(msg_create(M_KEY_REPEAT, key));
            }
        }
        else if (key_seen[key]) {
            key_seen[key] = false;
            mq_put(msg_create(M_KEY_UP, key));
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

/* Copy the source board to the destination, omitting any complete rows */
void collapse_full_rows(uint8_t src[32], uint8_t dest[32])
{
    int src_index, dest_index;

    dest_index = 31;
    for (src_index = 31; src_index >= 0; src_index--) {
        if (src[src_index] != 0xff)
            dest[dest_index--] = src[src_index];
    }
    while (dest_index >= 0)
        dest[dest_index--] = 0;
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

#define MOVE_LEFT 1
#define MOVE_RIGHT 2
#define ROTATE 3

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
    bool update_shape;
    uint8_t message;
    uint8_t action;

    HTpinsetup();
    HTsetup();
    set_up_keys();
    set_up_timer();
    HTbrightness(1);

    memset(board, 0, 32);
    memset(leds, 1, 32);
    HTsendscreen();

    shape_top = 0;
    shape_offset = 3;
    shape_rotation = rand() % 4;
    shape_selection = rand() % 7;
    shape_width = get_shape_width(shapes[shape_selection][shape_rotation]);
    memcpy(shape, shapes[shape_selection][shape_rotation], 4);
    offset_shape(shape, shape_offset);

    last_block_move_clock = clock_count;

    while (1) {
        action = 0;
        if (mq_get(&message)) {
            switch (msg_get_event(message)) {
            case M_KEY_DOWN:
            case M_KEY_REPEAT:
                switch (msg_get_param(message)) {
                case 0:
                    action = MOVE_LEFT;
                    break;
                case 1:
                    action = ROTATE;
                    break;
                case 2:
                    action = MOVE_RIGHT;
                }
            }
        }

        if ((clock_count - last_block_move_clock) > drop_interval) {
            last_block_move_clock = clock_count;
            if (test_collision(board, shape, shape_top + 1)) { /* block has fallen as far as it can */
                if (shape_top == 0)
                    break;

                collapse_full_rows(leds, board);

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
        if (action == MOVE_LEFT) {
            if (shape_offset > 0) {
                proposed_shape_offset = shape_offset - 1;
                if (proposed_shape_offset < 0)
                    proposed_shape_offset = 0;
                proposed_shape_rotation = shape_rotation;
                update_shape = true;
            }
        }
        else if (action == MOVE_RIGHT) {
            proposed_shape_offset = shape_offset + 1;
            if (proposed_shape_offset + shape_width > 8)
                proposed_shape_offset = 8 - shape_width;
            proposed_shape_rotation = shape_rotation;
            update_shape = true;
        }
        else if (action == ROTATE) {
            proposed_shape_rotation = (shape_rotation + 1) % 4;
            proposed_shape_offset = shape_offset;
            if (shape_offset + get_shape_width(shapes[shape_selection][proposed_shape_rotation]) < 9)
                update_shape = true;
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
}

