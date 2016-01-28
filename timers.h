#ifndef TIMERS_H
#define TIMERS_H

#include <stdint.h>

#define MAX_TIMERS 3

#ifndef MAX_TIMERS
#error MAX_TIMERS is undefined
#endif

#if MAX_TIMERS > 32
#error MAX_TIMERS must be <= 32
typedef uint32_t timer_bitfield;
#elif MAX_TIMERS > 16
typedef uint32_t timer_bitfield;
#elif MAX_TIMERS > 8
typedef uint16_t timer_bitfield;
#else
typedef uint8_t timer_bitfield;
#endif

typedef struct {
    uint16_t ms;
    uint16_t end;
} timer_t;

void set_timer(uint16_t ms, uint8_t n, bool recur);

void stop_timer(uint8_t n);

void handle_timers(void);

void init_timers(void);

extern volatile uint16_t clock_count;

#endif // TIMERS_H
