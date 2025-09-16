#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct Queue {
   uint8_t *buf;
   size_t   head;
   size_t   tail;
   size_t   nelem;
   size_t   size;
   size_t   mask_modulo;
} Queue;

#ifdef _MSC_VER
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))

/**
 * @brief round up to nearest power of two
 */
INLINE static size_t roundup_pow2(size_t num)
{
   if (!num)
      return 1;

   // my quick test shows that -O2 unrolls the loop (making it much faster) if sizeof(num) == 4, but not 8. -O3 does it in all cases 
   // to make sure, one could unroll it manually, but this is more readable
   num--;
   for (uint8_t i = 0; i < sizeof(num); i++) {
      num |= num >> (1 << i);
   }
   num++;
   return num;
}

void queue_init(Queue *q, size_t size)
{
   size_t size_pow2 = roundup_pow2(size);
   q->buf = malloc(size_pow2);
   q->size = size_pow2;
   q->mask_modulo = SIZE_MAX & (size_pow2 - 1);  // might be worth recomputing every time tho
   q->head = 0;
   q->tail = 0;
   q->nelem = 0;
}

size_t queue_push(Queue *q, const uint8_t *bytes, size_t count)
{
   size_t total = 0;

   if (q->head < q->tail || q->nelem == 0) {
      size_t count_r = MIN(q->size - q->tail, count);
      memcpy(&q->buf[q->tail], bytes, count_r);
      q->tail = (q->tail + count_r) & q->mask_modulo;
      total += count_r;
   }

   size_t count_l = MIN(q->head - q->tail, count - total);
   if (count_l > 0) {
      memcpy(&q->buf[q->tail], &bytes[total], count_l);
      q->tail += count_l;
      total += count_l;
   }

   q->nelem += total;

   return total;
}

size_t queue_pop(Queue *q, uint8_t *bytes, size_t count)
{
   if (q->nelem == 0)
      return 0;

   size_t total = 0;

   if (q->head >= q->tail) {
      size_t count_l = MIN(q->size - q->head, count);
      memcpy(bytes, &q->buf[q->head], count_l);
      q->head = (q->head + count_l) & q->mask_modulo;
      q->nelem -= count_l;
      total += count_l;
   }

   size_t count_r = MIN(q->tail - q->head, count - total);
   if (count_r) {
      memcpy(&bytes[total], &q->buf[q->head], count_r);
      q->head += count_r;
      q->nelem -= count_r;
      total += count_r;
   }

   return total;
}