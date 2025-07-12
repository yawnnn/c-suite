#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "jansson.h"

char *random_string(size_t length)
{
   const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   char      *str;

   str = malloc(length + 1);
   if (!str) {
      fprintf(stderr, "Memory allocation failed for random string.\n");
      exit(EXIT_FAILURE);
   }

   for (size_t i = 0; i < length; i++) {
      str[i] = charset[rand() % (sizeof(charset) - 1)];
   }
   str[length] = '\0';

   return str;
}

json_t *random_json_object(int depth, int max_depth)
{
   if (depth > max_depth) {
      switch (rand() % 4) {
         case 0:
            return json_string(random_string(20));
         case 1:
            return json_integer(rand() % 1000);
         case 2:
            return json_true();
         default:
            return json_null();
      }
   }

   json_t *obj = json_object();
   if (!obj) {
      fprintf(stderr, "Failed to create JSON object.\n");
      exit(EXIT_FAILURE);
   }

   for (int i = 0; i < 5; i++) { // Create 5 random key-value pairs
      char   *key;
      json_t *value;

      switch (rand() % 5) {
         case 0:
            value = json_string(random_string(20));
            break;
         case 1:
            value = json_integer(rand() % 1000);
            break;
         case 2:
            value = random_json_object(depth + 1, max_depth);
            break;
         case 3: {
            value = json_array();
            for (int j = 0; j < 5; j++) {
               json_array_append_new(value, json_string(random_string(20)));
            }
            break;
         }
         case 4: {
            value = json_array();
            for (int j = 0; j < 5; j++) {
               json_array_append_new(value, json_integer(rand() % 1000));
            }
            break;
         }
         default:
            value = json_null();
            break;
      }

      key = random_string(10);
      json_object_set_new(obj, key, value);
      free(key);
   }

   return obj;
}

void generate_random_json(const char *filename, int num_objects, int max_depth)
{
   FILE *file = fopen(filename, "w");
   if (!file) {
      fprintf(stderr, "Failed to open file '%s' for writing.\n", filename);
      exit(EXIT_FAILURE);
   }

   srand((unsigned int)time(NULL));
   json_set_alloc_funcs(malloc, free);

   fprintf(file, "{\"root\": [");

   for (int i = 0; i < num_objects; i++) {
      json_t *obj = random_json_object(1, max_depth);
      char   *json_str = json_dumps(obj, JSON_COMPACT);
      if (!json_str) {
         fprintf(stderr, "Failed to serialize JSON object.\n");
         exit(EXIT_FAILURE);
      }

      fprintf(file, "%s", json_str);
      free(json_str);
      json_decref(obj);

      if (i < num_objects - 1) {
         fprintf(file, ",");
      }
   }

   fprintf(file, "]}");
   fclose(file);

   printf("JSON successfully written to '%s'\n", filename);
}

#include "arena.h"
static Arena g_arena = {0};

void arena_init_()
{
   arena_init(&g_arena);
}

void arena_deinit_()
{
   arena_deinit(&g_arena);
}

void *arena_alloc_(size_t size)
{
   return arena_alloc(&g_arena, size);
}

void arena_free_(void *ptr) {}

typedef struct Bench {
   double mean, min, max, stddev;
} Bench;

#define NUM_CYCLES 5

void calc_bench(Bench *bench, clock_t *starts, clock_t *ends)
{
   double durations[NUM_CYCLES];

   memset(bench, 0, sizeof(*bench));

   for (int i = 0; i < NUM_CYCLES; ++i) {
      durations[i] = (double)(ends[i] - starts[i]) / CLOCKS_PER_SEC;
      bench->mean += durations[i];
   }

   bench->mean /= NUM_CYCLES;

   bench->min = bench->max = durations[0];
   for (int i = 1; i < NUM_CYCLES; ++i) {
      if (durations[i] < bench->min)
         bench->min = durations[i];
      if (durations[i] > bench->max)
         bench->max = durations[i];
   }

   for (int i = 0; i < NUM_CYCLES; ++i) {
      bench->stddev += (durations[i] - bench->mean) * (durations[i] - bench->mean);
   }
   bench->stddev = sqrt(bench->stddev / NUM_CYCLES);
}

void benchmark(
   Bench      *bench,
   const char *filename,
   void (*init_func)(),
   void (*deinit_func)(),
   void *(*alloc_func)(size_t),
   void (*free_func)(void *)
)
{
   json_set_alloc_funcs(alloc_func, free_func);

   clock_t starts[NUM_CYCLES], ends[NUM_CYCLES];

   for (int i = 0; i < NUM_CYCLES; i++) {
      starts[i] = clock();

      if (init_func)
         init_func();

      json_error_t error;
      json_t      *root = json_load_file(filename, 0, &error);

      if (!root)
         printf("Failed to load JSON file %s\n", error.text);
      else
         json_decref(root);

      if (deinit_func)
         deinit_func();

      ends[i] = clock();
   }

   calc_bench(bench, starts, ends);
}

void print_bench(Bench *bench, const char *what)
{
   printf("Results for `%s`: \n", what);
   printf("Average: %f secs\n", bench->mean);
   printf("Minimum: %f secs\n", bench->min);
   printf("Maximum: %f secs\n", bench->max);
   printf("Std Dev: %f secs\n", bench->stddev);
   printf("\n");
}

int main()
{
   FILE       *file;
   Bench       bench = {0};
   const char *filename = "benches/test.json";

   // generate big (~10MB) random JSON
   if ((file = fopen(filename, "r")) == NULL)
      generate_random_json(filename, 5000, 10);
   else
      fclose(file);

   benchmark(&bench, filename, NULL, NULL, malloc, free);
   print_bench(&bench, "malloc");

   benchmark(&bench, filename, arena_init_, arena_deinit_, arena_alloc_, arena_free_);
   print_bench(&bench, "arena");

   return 0;
}