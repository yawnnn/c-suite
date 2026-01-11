/*
 * Copyright (c) 2026 Alessandro Martone
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>

/**
 * @brief Single producer/single consumer lock-free queue
 */
typedef struct Queue {
   uint8_t       *buf;
   size_t         size;
   size_t         size_log2;  // fast divison: x / q->size == x >> q->size_log2
   _Atomic size_t head;
   _Atomic size_t tail;
} Queue;

/**
 * @brief initialize queue
 * 
 * @param[in,out] q queue
 * @param[in] buf buffer to be used
 * @param[in] size size of @p buf . must be a power of 2, otherwise some of it will not be used
 * 
 * @return actual size that will be used
 */
size_t queue_init(Queue *q, uint8_t *buf, size_t size);

/**
 * @brief push bytes into the queue
 * 
 * @param[in,out] q queue
 * @param[in] bytes bytes to be pushed
 * @param[in] count maximum number of bytes to be written
 * 
 * @return number of bytes written
 */
size_t queue_push(Queue *q, const uint8_t *bytes, size_t count);

/**
 * @brief pop bytes from the queue
 * 
 * @param[in,out] q queue
 * @param[out] bytes bytes read
 * @param[in] count maximum number of bytes to be read
 * 
 * @return number of bytes read
 */
size_t queue_pop(Queue *q, uint8_t *bytes, size_t count);

/**
 * @brief length of the queue
 * 
 * @param[in,out] q queue
 * 
 * @return number of bytes ready to be read from the queue
 */
size_t queue_len(Queue *q);

#endif /* __QUEUE_H__ */