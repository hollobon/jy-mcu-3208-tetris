#ifndef NUMBERS_H
#define NUMBERS_H

#include <avr/pgmspace.h>
#include "character.h"

const uint8_t _zero[] PROGMEM = {
    0b01111110,
    0b01000010,
    0b01111110
};

const uint8_t _one[] PROGMEM = {
    0b01111110
};

const uint8_t _two[] PROGMEM = {
    0b01111010,
    0b01001010,
    0b01001110,
};

const uint8_t _three[] PROGMEM = {
    0b01001010,
    0b01001010,
    0b01111110,
};

const uint8_t _four[] PROGMEM = {
    0b00011110,
    0b00010000,
    0b01111110,
};

const uint8_t _five[] PROGMEM = {
    0b01001110,
    0b01001010,
    0b01111010,
};

const uint8_t _six[] PROGMEM = {
    0b01111110,
    0b01001010,
    0b01111010,
};

const uint8_t _seven[] PROGMEM = {
    0b00000010,
    0b00000010,
    0b01111110,
};

const uint8_t _eight[] PROGMEM = {
    0b01111110,
    0b01001010,
    0b01111110,
};

const uint8_t _nine[] PROGMEM = {
    0b01001110,
    0b01001010,
    0b01111110,
};

character numbers[] = {
    CHARDEF(_zero),
    CHARDEF(_one),
    CHARDEF(_two),
    CHARDEF(_three),
    CHARDEF(_four),
    CHARDEF(_five),
    CHARDEF(_six),
    CHARDEF(_seven),
    CHARDEF(_eight),
    CHARDEF(_nine),
};

#endif // NUMBERS_H
