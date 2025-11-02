#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>

#include "queue.h"

#define QUEUE_SIZE 8
#define ITERATIONS 10000000
#define MASK       (QUEUE_SIZE - 1)

static uint8_t buffer[QUEUE_SIZE];
static Queue   q;

static void queue_reset(void)
{
   memset(buffer, 0xCD, QUEUE_SIZE);
   queue_init(&q, buffer, QUEUE_SIZE);
}

void test_basic_push_pop(void)
{
   queue_reset();

   uint8_t in[4] = {1, 2, 3, 4};
   uint8_t out[4] = {0};

   size_t w = queue_push(&q, in, 4);
   assert(w == 4);

   size_t r = queue_pop(&q, out, 4);
   assert(r == 4);
   assert(memcmp(in, out, 4) == 0);
   assert(memcmp(in, buffer, 4) == 0);

   printf("%s passed\n", __func__);
}

void test_pop_empty(void)
{
   queue_reset();

   uint8_t out[4];
   size_t  r = queue_pop(&q, out, 4);
   assert(r == 0);

   printf("%s passed\n", __func__);
}

void test_push_zero(void)
{
   queue_reset();

   uint8_t in[1] = {42};
   size_t  w = queue_push(&q, in, 0);
   assert(w == 0);

   printf("%s passed\n", __func__);
}

void test_wrap_exact(void)
{
   queue_reset();

   uint8_t in[QUEUE_SIZE];
   uint8_t out[QUEUE_SIZE];

   for (int i = 0; i < QUEUE_SIZE; i++)
      in[i] = (uint8_t)i;

   size_t w = queue_push(&q, in, QUEUE_SIZE);
   assert(w == QUEUE_SIZE);

   size_t r = queue_pop(&q, out, QUEUE_SIZE);
   assert(r == QUEUE_SIZE);
   assert(memcmp(in, out, QUEUE_SIZE) == 0);
   assert(memcmp(in, buffer, QUEUE_SIZE) == 0);

   printf("%s passed\n", __func__);
}

void test_wrap_partial(void)
{
   queue_reset();

   uint8_t in1[6] = {1, 2, 3, 4, 5, 6};
   uint8_t in2[4] = {7, 8, 9, 10};
   uint8_t out[10];

   assert(queue_push(&q, in1, 6) == 6);
   assert(queue_pop(&q, out, 4) == 4);
   assert(queue_push(&q, in2, 4) == 4);
   assert(queue_pop(&q, out, 6) == 6);

   uint8_t expect[6] = {5, 6, 7, 8, 9, 10};
   assert(memcmp(out, expect, 6) == 0);

   printf("%s passed\n", __func__);
}

void test_no_overwrite(void)
{
   queue_reset();

   uint8_t in[QUEUE_SIZE * 2];
   uint8_t out[QUEUE_SIZE];

   for (int i = 0; i < sizeof(in); i++)
      in[i] = (uint8_t)i;

   size_t w1 = queue_push(&q, in, QUEUE_SIZE);
   assert(w1 == QUEUE_SIZE);

   size_t w2 = queue_push(&q, in + QUEUE_SIZE, QUEUE_SIZE);
   assert(w2 == 0);  // must not overwrite

   size_t r = queue_pop(&q, out, QUEUE_SIZE);
   assert(r == QUEUE_SIZE);
   assert(memcmp(out, in, QUEUE_SIZE) == 0);

   printf("%s passed\n", __func__);
}

static void *producer(void *arg)
{
   uint8_t val = 0;
   for (size_t i = 0; i < ITERATIONS; i++) {
      while (queue_push(&q, &val, 1) == 0)
         ;  // spin
      val++;
   }

   printf("%s passed\n", __func__);

   return NULL;
}

static void *consumer(void *arg)
{
   uint8_t expected = 0;
   uint8_t val;

   for (size_t i = 0; i < ITERATIONS; i++) {
      while (queue_pop(&q, &val, 1) == 0)
         ;  // spin
      assert(val == expected);
      expected++;
   }

   printf("%s passed\n", __func__);

   return NULL;
}

void test_spsc_stress(void)
{
   queue_reset();

   pthread_t p, c;
   pthread_create(&p, NULL, producer, NULL);
   pthread_create(&c, NULL, consumer, NULL);

   pthread_join(p, NULL);
   pthread_join(c, NULL);

   printf("%s passed\n", __func__);
}

void test_long_run(void)
{
   queue_reset();

   uint8_t v = 0;
   uint8_t out;

   for (size_t i = 0; i < 10 * ITERATIONS; i++) {
      if (queue_push(&q, &v, 1) == 1)
         v++;

      if (queue_pop(&q, &out, 1) == 1)
         ;  // discard
   }

   printf("%s passed\n", __func__);
}

void test_manual_wrap_push(void)
{
   queue_reset();

   atomic_store_explicit(&q.head, 2, memory_order_release);
   atomic_store_explicit(&q.tail, 6, memory_order_release);

   uint8_t in[4] = {1, 2, 3, 4};
   uint8_t out[4] = {0};

   size_t w = queue_push(&q, in, 4);
   assert(w == 4);

   queue_pop(&q, out, 4);
   size_t r = queue_pop(&q, out, 4);
   assert(r == 4);

   assert(memcmp(in, out, 4) == 0);

   printf("%s passed\n", __func__);
}

void test_manual_wrap_pop(void)
{
   queue_reset();

   atomic_store_explicit(&q.head, 6, memory_order_release);
   atomic_store_explicit(&q.tail, 10, memory_order_release);

   buffer[6] = 1;
   buffer[7] = 2;
   buffer[0] = 3;
   buffer[1] = 4;

   uint8_t out[4];

   size_t r = queue_pop(&q, out, 4);
   assert(r == 4);

   uint8_t expect[4] = {1, 2, 3, 4};
   assert(memcmp(out, expect, 4) == 0);

   printf("%s passed\n", __func__);
}

void test_manual_exact_boundary(void)
{
   queue_reset();

   atomic_store_explicit(&q.head, 0, memory_order_release);
   atomic_store_explicit(&q.tail, QUEUE_SIZE, memory_order_release);

   uint8_t in[QUEUE_SIZE];
   uint8_t out[QUEUE_SIZE] = {0};

   for (size_t i = 0; i < QUEUE_SIZE; i++) {
      in[i] = (uint8_t)(i + 10);
      buffer[i] = (uint8_t)(i + 10);
   }

   size_t w = queue_push(&q, out, QUEUE_SIZE);
   assert(w == 0);

   size_t r = queue_pop(&q, out, QUEUE_SIZE);
   assert(r == QUEUE_SIZE);

   assert(memcmp(in, out, QUEUE_SIZE) == 0);

   printf("%s passed\n", __func__);
}

void test_size_t_wrap_push(void)
{
   queue_reset();

   atomic_store_explicit(&q.head, SIZE_MAX - 6, memory_order_release);
   atomic_store_explicit(&q.tail, SIZE_MAX - 6 + 4, memory_order_release);

   uint8_t in[4] = {1, 2, 3, 4};
   uint8_t out[4] = {0};

   size_t w = queue_push(&q, in, 4);
   assert(w == 4);

   queue_pop(&q, out, 4);
   size_t r = queue_pop(&q, out, 4);
   assert(r == 4);

   assert(memcmp(in, out, 4) == 0);

   printf("%s passed\n", __func__);
}

void test_size_t_wrap_pop(void)
{
   queue_reset();

   atomic_store_explicit(&q.head, SIZE_MAX - 3, memory_order_release);
   atomic_store_explicit(&q.tail, SIZE_MAX - 3 + 4, memory_order_release);

   buffer[5] = 10;
   buffer[6] = 11;
   buffer[7] = 12;
   buffer[0] = 13;
   buffer[1] = 14;

   uint8_t out[5];

   size_t r = queue_pop(&q, out, 5);
   assert(r == 5);

   uint8_t expect[5] = {10, 11, 12, 13, 14};
   assert(memcmp(out, expect, 5) == 0);

   printf("%s passed\n", __func__);
}

void test_nloop_wrap(void)
{
   queue_reset();

   atomic_store_explicit(&q.head, SIZE_MAX - 3, memory_order_release);
   atomic_store_explicit(&q.tail, SIZE_MAX - 3 + 4, memory_order_release);

   uint8_t in[5] = {10, 11, 12, 13, 14};
   uint8_t out[5];

   size_t w = queue_push(&q, in, 5);
   assert(w == 5);

   size_t r = queue_pop(&q, out, 5);
   assert(r == 5);
   assert(memcmp(in, out, 5) == 0);

   printf("%s passed\n", __func__);
}

void test_size_t_wrap_interleaved(void)
{
   queue_reset();

   atomic_store_explicit(&q.head, SIZE_MAX - 1, memory_order_release);
   atomic_store_explicit(&q.tail, SIZE_MAX - 1, memory_order_release);

   uint8_t in[2] = {42, 43};
   uint8_t out[2];

   assert(queue_push(&q, in, 1) == 1);
   assert(queue_push(&q, in, 1) == 1);
   assert(queue_pop(&q, out, 1) == 1);
   assert(out[0] == 42);
   assert(queue_pop(&q, out, 1) == 1);
   assert(out[0] == 43);

   assert(queue_push(&q, in, 2) == 2);
   assert(queue_pop(&q, out, 2) == 2);
   assert(memcmp(in, out, 2) == 0);

   printf("%s passed\n", __func__);
}

int main(void)
{
   test_basic_push_pop();
   test_pop_empty();
   test_push_zero();
   test_wrap_exact();
   test_wrap_partial();
   test_no_overwrite();
   test_spsc_stress();
   //test_long_run();
   test_manual_wrap_push();
   test_manual_wrap_pop();
   test_manual_exact_boundary();

   printf("All tests passed\n");
   return 0;
}
