#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef _MSC_VER
   #define INLINE __inline
#elif defined(__STDC__) && __STDC_VERSION__ >= 199901L
   #define INLINE inline
#else
   #define INLINE
#endif

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
   #define ATOMIC _Atomic
#else
   #define ATOMIC 
#endif

typedef struct Queue {
   uint8_t *buf;
   size_t   size;
   size_t   size_mask;  // fast modulo: x % q->size == x & q->size_mask
   size_t   size_log2;  // fast divison: x / q->size == x >> q->size_log2
   ATOMIC size_t   head;
   ATOMIC size_t   tail;
} Queue;

#if __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
   #include <stdatomic.h>

static INLINE size_t atomic_load_acquire(const _Atomic size_t *p)
{
   return atomic_load_explicit(p, memory_order_acquire);
}

static INLINE void atomic_store_release(_Atomic size_t *p, size_t v)
{
   atomic_store_explicit(p, v, memory_order_release);
}

#elif defined(__GNUC__) /* Works for GCC and Clang */
static INLINE size_t atomic_load_acquire(const size_t *p)
{
   size_t v = *p;
   __atomic_thread_fence(__ATOMIC_ACQUIRE);
   return v;
}

static INLINE void atomic_store_release(size_t *p, size_t v)
{
   __atomic_thread_fence(__ATOMIC_RELEASE);
   *p = v;
}

#elif defined(_MSC_VER)
   #include <intrin.h>

static INLINE size_t atomic_load_acquire(const size_t *p)
{
   size_t v = *p;
   MemoryBarrier();
   return v;
}

static INLINE void atomic_store_release(size_t *p, size_t v)
{
   MemoryBarrier();
   *p = v;
}

#else
   /* LAST RESORT — not safe between multiple threads — only SC/SC use */
   #warning "No atomic support: falling back to volatile operations."

static INLINE size_t atomic_load_acquire(const volatile size_t *p)
{
   return *p;
}

static INLINE void atomic_store_release(volatile size_t *p, size_t v)
{
   *p = v;
}
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

size_t queue_init(Queue *q, uint8_t *buf, size_t size)
{
   q->size_log2 = ilog2_pow2(size);
   q->size = 1 << q->size_log2;
   q->size_mask = q->size - 1;
   q->buf = buf;
   q->head = 0;
   q->tail = 0;

   return q->size;
}

size_t queue_push(Queue *q, const uint8_t *bytes, size_t count)
{
   size_t tail_pos;
   size_t total = 0;
   size_t head = atomic_load_acquire(&q->head);
   size_t tail = q->tail;
   size_t head_nloop = head >> q->size_log2;
   size_t tail_nloop = q->tail >> q->size_log2;

   if (head_nloop == tail_nloop) {
      tail_pos = tail & q->size_mask;
      size_t count_r = min(q->size - tail_pos, count);
      memcpy(&q->buf[tail_pos], &bytes[total], count_r);
      tail += count_r;
      total += count_r;
   }

   tail_pos = tail & q->size_mask;
   size_t count_l = min((head & q->size_mask) - tail_pos, count - total);
   if (count_l > 0) {
      memcpy(&q->buf[tail_pos], &bytes[total], count_l);
      tail += count_l;
      total += count_l;
   }

   atomic_store_release(&q->tail, tail);

   return total;
}

size_t queue_pop(Queue *q, uint8_t *bytes, size_t count)
{
   size_t head_pos;
   size_t total = 0;
   size_t head = q->head;
   size_t tail = atomic_load_acquire(&q->tail);
   size_t head_nloop = q->head >> q->size_log2;
   size_t tail_nloop = tail >> q->size_log2;

   if (head_nloop != tail_nloop) {
      head_pos = head & q->size_mask;
      size_t count_l = min(q->size - head_pos, count);
      memcpy(&bytes[total], &q->buf[head_pos], count_l);
      head += count_l;
      total += count_l;
   }

   head_pos = head & q->size_mask;
   size_t count_r = min((tail & q->size_mask) - head_pos, count - total);
   if (count_r) {
      memcpy(&bytes[total], &q->buf[head_pos], count_r);
      head += count_r;
      total += count_r;
   }

   atomic_store_release(&q->head, head);

   return total;
}