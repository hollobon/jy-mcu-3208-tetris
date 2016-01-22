/* Tiny ring-buffer based message queue
 *
 * Copyright (C) Pete Hollobon 2015
 */

#include "mq.h"
#include <stdbool.h>
#include <stdint.h>
#include <util/atomic.h>

volatile message_t _mq[MQ_SIZE];
volatile uint8_t _mq_front = 0;
volatile uint8_t _mq_back = 0;

bool mq_put(message_t value)
{
    if (mq_space < 1)
        return false;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        _mq[_mq_front] = value;
        _mq_front = (_mq_front + 1) % MQ_SIZE;
    }
    return true;
}

bool mq_get(message_t *value)
{
    if (mq_empty)
        return false;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        *value = _mq[_mq_back];
        _mq_back = (_mq_back + 1) % MQ_SIZE;
    }
    return true;
}
