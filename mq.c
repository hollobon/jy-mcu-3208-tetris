/* Tiny ring-buffer based message queue
 *
 * Copyright (C) Pete Hollobon 2015
 */

#include "mq.h"
#include <stdbool.h>
#include <stdint.h>

uint8_t _mq[MQ_SIZE];
uint8_t _mq_front = 0;
uint8_t _mq_back = 0;

bool mq_put(uint8_t value)
{
    if (mq_space < 1)
        return false;

    _mq[_mq_front] = value;
    _mq_front = (_mq_front + 1) % MQ_SIZE;
    return true;
}

bool mq_get(uint8_t *value)
{
    if (mq_empty)
        return false;

    *value = _mq[_mq_back];
    _mq_back = (_mq_back + 1) % MQ_SIZE;
    return true;
}
