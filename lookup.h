#ifndef LOOKUP_H
#define LOOKUP_H

#include <avr/pgmspace.h>

const uint8_t _cos_s16_r1_15[] PROGMEM = {1, 1, 2, 2, 3, 4, 5, 7, 8, 9, 11, 12, 13, 14, 14, 15};
#define cos_s16_r1_15(n) pgm_read_byte(&(_cos_s16_r1_15[(n)]))
#endif // LOOKUP_H
