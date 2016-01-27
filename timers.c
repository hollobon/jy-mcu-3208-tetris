#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <util/atomic.h>
#include "timers.h"
#include "mq.h"

static timer_t timers[MAX_TIMERS];
static timer_bitfield timers_active = 0;
static timer_bitfield timers_recur = 0;

volatile uint16_t clock_count = 0;

void set_timer(uint16_t ms, uint8_t n, bool recur)
{
    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        if (recur)
            timers_recur |= 1 << n;
        else
            timers_recur &= ~(1 << n);

        timers[n].ms = ms;
        timers[n].end = clock_count + ms;
        timers_active |= 1 << n;
    }
}

void stop_timer(uint8_t n)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        timers_active &= ~(1 << n);
    }
}

inline void handle_timers(void)
{
    for (int timer = 0; timer < MAX_TIMERS; timer++) {
        if (timers_active & (1 << timer) && clock_count == timers[timer].end) {
            if (!(timers_recur & (1 << timer)))
                stop_timer(timer);
            else
                timers[timer].end = clock_count + timers[timer].ms;
            mq_put(msg_create(M_TIMER, timer));
        }
    }

    clock_count++;
}

void init_timers(void)
{
    memset(timers, 0, MAX_TIMERS);
}
