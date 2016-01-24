#ifndef TUNE_H
#define TUNE_H
#include <stdint.h>
#include <avr/pgmspace.h>

/* Autogenerated by create_music.py - do not edit */

typedef struct {
    uint8_t note: 5, time: 3;
} _note;

typedef union {
    _note;
    uint8_t decode;
} note;

const uint8_t note_clocks[] = {
    24, // E5
    19, // G#5
    18, // A5
    16, // B5
    15, // C6
    13, // D6
    12, // E6
    11, // F6
    10, // G6
    9, // A6
    53, // D4
    47, // E4
    38, // G#4
    36, // A4
    32, // B4
    30, // C5
    27, // D5
};

const _note tune[] PROGMEM = {
    {6, 1}, // E_6
    {11, 1}, // E_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {6, 0}, // E_6
    {5, 0}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {4, 1}, // C_6
    {6, 1}, // E_6
    {13, 1}, // A_4
    {5, 1}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {12, 1}, // Gs_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {11, 1}, // E_4
    {6, 1}, // E_6
    {11, 1}, // E_4
    {4, 1}, // C_6
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 3}, // A_4
    {13, 1}, // A_4
    {10, 1}, // D_4
    {5, 3}, // D_6
    {7, 1}, // F_6
    {9, 1}, // A_6
    {10, 1}, // D_4
    {8, 1}, // G_6
    {7, 1}, // F_6
    {6, 1}, // E_6
    {13, 3}, // A_4
    {4, 1}, // C_6
    {6, 1}, // E_6
    {13, 1}, // A_4
    {5, 1}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {12, 1}, // Gs_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {11, 1}, // E_4
    {6, 1}, // E_6
    {11, 1}, // E_4
    {4, 1}, // C_6
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 3}, // A_4
    {13, 1}, // A_4
    {6, 1}, // E_6
    {11, 1}, // E_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {6, 0}, // E_6
    {5, 0}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {4, 1}, // C_6
    {6, 1}, // E_6
    {13, 1}, // A_4
    {5, 1}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {12, 1}, // Gs_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {11, 1}, // E_4
    {6, 1}, // E_6
    {11, 1}, // E_4
    {4, 1}, // C_6
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 3}, // A_4
    {13, 1}, // A_4
    {10, 1}, // D_4
    {5, 3}, // D_6
    {7, 1}, // F_6
    {9, 1}, // A_6
    {10, 1}, // D_4
    {8, 1}, // G_6
    {7, 1}, // F_6
    {6, 1}, // E_6
    {13, 3}, // A_4
    {4, 1}, // C_6
    {6, 1}, // E_6
    {13, 1}, // A_4
    {5, 1}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {12, 1}, // Gs_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {11, 1}, // E_4
    {6, 1}, // E_6
    {11, 1}, // E_4
    {4, 1}, // C_6
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 3}, // A_4
    {13, 1}, // A_4
    {0, 1}, // E_5
    {11, 3}, // E_4
    {11, 1}, // E_4
    {15, 1}, // C_5
    {11, 3}, // E_4
    {11, 1}, // E_4
    {16, 1}, // D_5
    {11, 3}, // E_4
    {11, 1}, // E_4
    {14, 1}, // B_4
    {11, 3}, // E_4
    {11, 1}, // E_4
    {15, 1}, // C_5
    {11, 3}, // E_4
    {11, 1}, // E_4
    {13, 1}, // A_4
    {11, 3}, // E_4
    {11, 1}, // E_4
    {12, 1}, // Gs_4
    {11, 3}, // E_4
    {11, 3}, // E_4
    {11, 3}, // E_4
    {11, 1}, // E_4
    {0, 1}, // E_5
    {11, 3}, // E_4
    {11, 1}, // E_4
    {15, 1}, // C_5
    {11, 3}, // E_4
    {11, 1}, // E_4
    {16, 1}, // D_5
    {11, 3}, // E_4
    {11, 1}, // E_4
    {14, 1}, // B_4
    {11, 3}, // E_4
    {11, 1}, // E_4
    {15, 1}, // C_5
    {11, 1}, // E_4
    {0, 1}, // E_5
    {11, 1}, // E_4
    {2, 1}, // A_5
    {11, 3}, // E_4
    {11, 1}, // E_4
    {1, 1}, // Gs_5
    {11, 3}, // E_4
    {11, 3}, // E_4
    {11, 3}, // E_4
    {11, 1}, // E_4
    {6, 1}, // E_6
    {11, 1}, // E_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {6, 0}, // E_6
    {5, 0}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {4, 1}, // C_6
    {6, 1}, // E_6
    {13, 1}, // A_4
    {5, 1}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {12, 1}, // Gs_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {11, 1}, // E_4
    {6, 1}, // E_6
    {11, 1}, // E_4
    {4, 1}, // C_6
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 3}, // A_4
    {13, 1}, // A_4
    {10, 1}, // D_4
    {5, 3}, // D_6
    {7, 1}, // F_6
    {9, 1}, // A_6
    {10, 1}, // D_4
    {8, 1}, // G_6
    {7, 1}, // F_6
    {6, 1}, // E_6
    {13, 3}, // A_4
    {4, 1}, // C_6
    {6, 1}, // E_6
    {13, 1}, // A_4
    {5, 1}, // D_6
    {4, 1}, // C_6
    {3, 1}, // B_5
    {12, 1}, // Gs_4
    {3, 1}, // B_5
    {4, 1}, // C_6
    {5, 1}, // D_6
    {11, 1}, // E_4
    {6, 1}, // E_6
    {11, 1}, // E_4
    {4, 1}, // C_6
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 1}, // A_4
    {2, 1}, // A_5
    {13, 3}, // A_4
};
#define TUNE_LENGTH 229
#define NOTE_LENGTH_MULTIPLIER 48

#endif // TUNE_H
