/* Tetris-like falling blocks game for JY-MCY 3208 "Lattice Clock"
 * 
 * Copyright (C) Pete Hollobon 2015
 */

#ifndef TETRIS_H
#define TETRIS_H

#include <stdint.h>

/* Array of shapes. A shape is represented by four bytes, one byte per row.
   Each shape includes four rotations. 
*/
uint8_t shapes[7][4][4] = {
    /* line */
    {{0b11110000, 0b00000000, 0b00000000, 0b00000000},
     {0b10000000, 0b10000000, 0b10000000, 0b10000000},
     {0b11110000, 0b00000000, 0b00000000, 0b00000000},
     {0b10000000, 0b10000000, 0b10000000, 0b10000000}},
    /* box */
    {{0b11000000, 0b11000000, 0b00000000, 0b00000000},
     {0b11000000, 0b11000000, 0b00000000, 0b00000000},
     {0b11000000, 0b11000000, 0b00000000, 0b00000000},
     {0b11000000, 0b11000000, 0b00000000, 0b00000000}},
    /* L left */
    {{0b10000000, 0b11100000, 0b00000000, 0b00000000},
     {0b11000000, 0b10000000, 0b10000000, 0b00000000},
     {0b11100000, 0b00100000, 0b00000000, 0b00000000},
     {0b01000000, 0b01000000, 0b11000000, 0b00000000}},
    /* L right */
    {{0b00100000, 0b11100000, 0b00000000, 0b00000000},
     {0b10000000, 0b10000000, 0b11000000, 0b00000000},
     {0b11100000, 0b10000000, 0b00000000, 0b00000000},
     {0b11000000, 0b01000000, 0b01000000, 0b00000000}},
    /* T */
    {{0b01000000, 0b11100000, 0b00000000, 0b00000000},
     {0b10000000, 0b11000000, 0b10000000, 0b00000000},
     {0b11100000, 0b01000000, 0b00000000, 0b00000000},
     {0b01000000, 0b11000000, 0b01000000, 0b00000000}},
    /* S left */
    {{0b10000000, 0b11000000, 0b01000000, 0b00000000},
     {0b01100000, 0b11000000, 0b00000000, 0b00000000},
     {0b10000000, 0b11000000, 0b01000000, 0b00000000},
     {0b01100000, 0b11000000, 0b00000000, 0b00000000}},
    /* S right */
    {{0b01000000, 0b11000000, 0b10000000, 0b00000000},
     {0b11000000, 0b01100000, 0b00000000, 0b00000000},
     {0b01000000, 0b11000000, 0b10000000, 0b00000000},
     {0b11000000, 0b01100000, 0b00000000, 0b00000000}},
};

void overlay_shape(uint8_t src[32], uint8_t dest[32], uint8_t shape[4], uint8_t shape_top);

void offset_shape(uint8_t shape[4], uint8_t n);

#endif  /* TETRIS_H */
