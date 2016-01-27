#include <stdbool.h>
#include "music.h"
#include "tune.h"
#include "timers.h"

static uint8_t tune_location = 0;

void stop_music(void)
{
    // no interrupt on timer 2 compare match
    TIMSK &= ~_BV(OCIE2);
    stop_timer(1);
}

void start_music(void)
{
    tune_location = 0;

    // interrupt on timer 2 compare match
    TIMSK |= _BV(OCIE2);

    set_timer(100, 1, false);
}

void handle_music(void)
{
    note n;
    n.decode = pgm_read_byte(&(tune[tune_location]));

    OCR2 = note_clocks[n.note];
    set_timer((n.time + 1) * 2 * NOTE_LENGTH_MULTIPLIER, 1, false);

    tune_location++;
    if (tune_location > TUNE_LENGTH)
        tune_location = 0;
}

