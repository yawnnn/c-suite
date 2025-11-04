#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Queue {
   uint8_t *buf;
   size_t   size;
   size_t   size_mask;  // fast modulo: x % q->size == x & q->size_mask
   size_t   size_log2;  // fast divison: x / q->size == x >> q->size_log2
   size_t   head;
   size_t   tail;
} Queue;

#ifdef _MSC_VER
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

INLINE static min(size_t a, size_t b)
{
   return a < b ? a : b;
}

/**
 * @brief log2(num) when num is power of 2
 */
INLINE static size_t ilog2_pow2(size_t num)
{
   // alternatively gcc has __builtin_ctz and MSVC has _BitScanForward
   size_t log = 0;

   while (num > 1) {
      num >>= 1;
      log++;
   }

   return log;
}

bool queue_init(Queue *q, uint8_t *buf, size_t size)
{
   if (size & (size - 1))
      return false;

   q->buf = buf;
   q->size = size;
   q->size_mask = q->size - 1;
   q->size_log2 = ilog2_pow2(q->size);
   q->head = 0;
   q->tail = 0;

   return true;
}

size_t queue_push(Queue *q, const uint8_t *bytes, size_t count)
{
   size_t tail_pos;
   size_t total = 0;
   size_t head_nloop = q->head >> q->size_log2;
   size_t tail_nloop = q->tail >> q->size_log2;

   if (head_nloop == tail_nloop) {
      tail_pos = q->tail & q->size_mask;
      size_t count_r = min(q->size - tail_pos, count);
      memcpy(&q->buf[tail_pos], &bytes[total], count_r);
      q->tail += count_r;
      total += count_r;
   }

   tail_pos = q->tail & q->size_mask;
   size_t count_l = min((q->head & q->size_mask) - tail_pos, count - total);
   if (count_l > 0) {
      memcpy(&q->buf[tail_pos], &bytes[total], count_l);
      q->tail += count_l;
      total += count_l;
   }

   return total;
}

size_t queue_pop(Queue *q, uint8_t *bytes, size_t count)
{
   size_t head_pos;
   size_t total = 0;
   size_t head_nloop = q->head >> q->size_log2;
   size_t tail_nloop = q->tail >> q->size_log2;

   if (head_nloop != tail_nloop) {
      head_pos = q->head & q->size_mask;
      size_t count_l = min(q->size - head_pos, count);
      memcpy(&bytes[total], &q->buf[head_pos], count_l);
      q->head += count_l;
      total += count_l;
   }

   head_pos = q->head & q->size_mask;
   size_t count_r = min((q->tail & q->size_mask) - head_pos, count - total);
   if (count_r) {
      memcpy(&bytes[total], &q->buf[head_pos], count_r);
      q->head += count_r;
      total += count_r;
   }

   return total;
}