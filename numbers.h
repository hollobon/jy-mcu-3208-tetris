#ifndef NUMBERS_H
#define NUMBERS_H

#define CHARDEF(bitmap) {bitmap, sizeof(bitmap)}

typedef struct {
    uint8_t *bitmap;
    uint8_t columns;
} character;

uint8_t _zero[] = {
    0b01111110,
    0b01000010,
    0b01111110
};

uint8_t _one[] = {
    0b01111110
};

uint8_t _two[] = {
    0b01111010,
    0b01001010,
    0b01001110,
};

uint8_t _three[] = {
    0b01001010,
    0b01001010,
    0b01111110,
};

uint8_t _four[] = {
    0b00011110,
    0b00010000,
    0b01111110,
};

uint8_t _five[] = {
    0b01001110,
    0b01001010,
    0b01111010,
};

uint8_t _six[] = {
    0b01111110,
    0b01001010,
    0b01111010,
};

uint8_t _seven[] = {
    0b00000010,
    0b00000010,
    0b01111110,
};

uint8_t _eight[] = {
    0b01111110,
    0b01001010,
    0b01111110,
};

uint8_t _nine[] = {
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
