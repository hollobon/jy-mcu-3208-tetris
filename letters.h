#ifndef LETTERS_H
#define LETTERS_H

#include <avr/pgmspace.h>
#include "character.h"

const uint8_t _a[] PROGMEM = {
    0b01111100,
    0b00010010,
    0b01111100,
};

const uint8_t _b[] PROGMEM = {
    0b01111110,
    0b01001010,
    0b00110100,
};

const uint8_t _c[] PROGMEM = {
    0b00111100,
    0b01000010,
    0b01000010,
};

const uint8_t _d[] PROGMEM = {
    0b01111110,
    0b01000010,
    0b00111100,
};

const uint8_t _e[] PROGMEM = {
    0b01111110,
    0b01001010,
    0b01001010,
};

const uint8_t _f[] PROGMEM = {
    0b01111110,
    0b00001010,
    0b00001010,
};

const uint8_t _g[] PROGMEM = {
    0b00111100,
    0b01000010,
    0b00111010,
};

const uint8_t _h[] PROGMEM = {
    0b01111110,
    0b00001000,
    0b01111110,
};

const uint8_t _i[] PROGMEM = {
    0b01000010,
    0b01111110,
    0b01000010,
};

const uint8_t _j[] PROGMEM = {
    0b00100010,
    0b01000010,
    0b00111110,
};

const uint8_t _k[] PROGMEM = {
    0b01111110,
    0b00001000,
    0b01110110,
};

const uint8_t _l[] PROGMEM = {
    0b01111110,
    0b01000000,
    0b01000000,
};

const uint8_t _m[] PROGMEM = {
    0b01111110,
    0b00001100,
    0b00010000,
    0b00001100,
    0b01111110,
};

const uint8_t _n[] PROGMEM = {
    0b01111110,
    0b00001000,
    0b00010000,
    0b01111110,
};

const uint8_t _o[] PROGMEM = {
    0b00111100,
    0b01000010,
    0b00111100,
};

const uint8_t _p[] PROGMEM = {
    0b01111110,
    0b00001010,
    0b00001110,
};

const uint8_t _q[] PROGMEM = {
    0b10111100,
    0b01000010,
    0b00111100,
};

const uint8_t _r[] PROGMEM = {
    0b01111110,
    0b00001010,
    0b01110100,
};

const uint8_t _s[] PROGMEM = {
    0b01000100,
    0b01001010,
    0b00110010,
};

const uint8_t _t[] PROGMEM = {
    0b00000010,
    0b01111110,
    0b00000010,
};

const uint8_t _u[] PROGMEM = {
    0b00111110,
    0b01000000,
    0b00111110,
};

const uint8_t _v[] PROGMEM = {
    0b00011110,
    0b01100000,
    0b00011110,
};

const uint8_t _w[] PROGMEM = {
    0b00111110,
    0b01000000,
    0b00110000,
    0b01000000,
    0b00111110,
};

const uint8_t _x[] PROGMEM = {
    0b01100110,
    0b00011000,
    0b01100110,
};

const uint8_t _y[] PROGMEM = {
    0b00000110,
    0b01111000,
    0b00000110,
};

const uint8_t _z[] PROGMEM = {
    0b01100010,
    0b01011010,
    0b01000110,
};

const character letters[] = {
    CHARDEF(_a),
    CHARDEF(_b),
    CHARDEF(_c),
    CHARDEF(_d),
    CHARDEF(_e),
    CHARDEF(_f),
    CHARDEF(_g),
    CHARDEF(_h),
    CHARDEF(_i),
    CHARDEF(_j),
    CHARDEF(_k),
    CHARDEF(_l),
    CHARDEF(_m),
    CHARDEF(_n),
    CHARDEF(_o),
    CHARDEF(_p),
    CHARDEF(_q),
    CHARDEF(_r),
    CHARDEF(_s),
    CHARDEF(_t),
    CHARDEF(_u),
    CHARDEF(_v),
    CHARDEF(_w),
    CHARDEF(_x),
    CHARDEF(_y),
    CHARDEF(_z),
};

#endif // LETTERS_H
