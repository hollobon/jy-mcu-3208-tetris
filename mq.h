/* Tiny ring-buffer based message queue
 *
 * Copyright (C) Pete Hollobon 2015
 */

#ifndef MQ_H
#define MQ_H

#include <stdbool.h>
#include <stdint.h>

#define MQ_SIZE 20

#define mq_empty (_mq_front == _mq_back)
#define mq_used ((_mq_front - _mq_back) % MQ_SIZE)
#define mq_space (MQ_SIZE - mq_used)

bool mq_put(uint8_t value);
bool mq_get(uint8_t *value);

#define msg_create(event, param) (param << 4 | (event & 0x0F))
#define msg_get_event(msg) (msg & 0x0F)
#define msg_get_param(msg) (msg >> 4)

#endif /* MQ_H */
