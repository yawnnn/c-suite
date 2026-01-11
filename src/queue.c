/*
 * Copyright (c) 2026 Alessandro Martone
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include <stdlib.h>
#include <string.h>

#include "queue.h"

#ifdef _MSC_VER
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

INLINE static size_t min(size_t a, size_t b)
{
   return a < b ? a : b;
}

/**
 * @brief log2(num) when num is power of 2
 */
INLINE static size_t ilog2_pow2(size_t num)
{
   size_t log;

   for (log = 0; num > 1; log++) {
      num /= 2;
   }

   return log;
}

/**
 * @brief fast division of @p n by size (which is a power of 2)
 */
INLINE static size_t queue_div(Queue *q, size_t n)
{
   return n >> q->size_log2;
}

/**
 * @brief fast modulo of @p n by size (which is a power of 2)
 */
INLINE static size_t queue_mod(Queue *q, size_t n)
{
   return n & (q->size - 1);
}

size_t queue_init(Queue *q, uint8_t *buf, size_t size)
{
   q->size_log2 = ilog2_pow2(size);
   q->size = q->size_log2 ? 1 << q->size_log2 : 0;
   q->buf = buf;
   q->head = 0;
   q->tail = 0;

   return q->size;
}

size_t queue_push(Queue *q, const uint8_t *bytes, size_t count)
{
   size_t tail_pos, total = 0;
   size_t head = atomic_load_explicit(&q->head, memory_order_acquire);
   size_t tail = atomic_load_explicit(&q->tail, memory_order_relaxed);
   size_t head_nloop = queue_div(q, head);
   size_t tail_nloop = queue_div(q, tail);

   if (head_nloop == tail_nloop) {
      tail_pos = queue_mod(q, tail);
      size_t count_r = min(q->size - tail_pos, count);
      memcpy(&q->buf[tail_pos], &bytes[total], count_r);
      tail += count_r;
      total += count_r;
   }

   tail_pos = queue_mod(q, tail);
   size_t count_l = min(
      queue_mod(q, head) - tail_pos,
      count - total
   );  // lhs can underflow only when rhs is 0, so its okay
   if (count_l > 0) {
      memcpy(&q->buf[tail_pos], &bytes[total], count_l);
      tail += count_l;
      total += count_l;
   }

   atomic_store_explicit(&q->tail, tail, memory_order_release);

   return total;
}

size_t queue_pop(Queue *q, uint8_t *bytes, size_t count)
{
   size_t head_pos, total = 0;
   size_t head = atomic_load_explicit(&q->head, memory_order_relaxed);
   size_t tail = atomic_load_explicit(&q->tail, memory_order_acquire);
   size_t head_nloop = queue_div(q, head);
   size_t tail_nloop = queue_div(q, tail);

   if (head_nloop != tail_nloop) {
      head_pos = queue_mod(q, head);
      size_t count_l = min(q->size - head_pos, count);
      memcpy(&bytes[total], &q->buf[head_pos], count_l);
      head += count_l;
      total += count_l;
   }

   head_pos = queue_mod(q, head);
   size_t count_r = min(
      queue_mod(q, tail) - head_pos,
      count - total
   );  // lhs can underflow only when rhs is 0, so its okay
   if (count_r) {
      memcpy(&bytes[total], &q->buf[head_pos], count_r);
      head += count_r;
      total += count_r;
   }

   atomic_store_explicit(&q->head, head, memory_order_release);

   return total;
}

size_t queue_len(Queue *q)
{
   size_t head = atomic_load_explicit(&q->head, memory_order_acquire);
   size_t tail = atomic_load_explicit(&q->tail, memory_order_acquire);
   size_t head_nloop = queue_div(q, head);
   size_t tail_nloop = queue_div(q, tail);

   if (head_nloop == tail_nloop)
      return tail - head;
   return q->size - (queue_mod(q, head) - queue_mod(q, tail));
}