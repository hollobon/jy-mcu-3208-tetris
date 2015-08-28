#ifndef CHARACTER_H
#define CHARACTER_H

#define CHARDEF(bitmap) {bitmap, sizeof(bitmap)}

typedef struct {
    uint8_t *bitmap;
    uint8_t columns;
} character;

#endif // CHARACTER_H
