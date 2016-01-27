#ifndef CHARACTER_H
#define CHARACTER_H

#define CHARDEF(bitmap) {(bitmap), sizeof(bitmap)}

typedef struct {
    const uint8_t *bitmap;
    const uint8_t columns;
} character;

#endif // CHARACTER_H
