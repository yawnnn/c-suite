#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "jansson.h"

#include "arena.h"

static Arena g_arena = {0};

void _arena_init() {
   arena_init(&g_arena, 8192);
}

void _arena_deinit() {
   arena_deinit(&g_arena);
}

void *_arena_alloc(size_t size) {
   return arena_alloc(&g_arena, size);
}

void _arena_free(void *ptr) {}

#define NUM_CYCLES 5

// Function to generate a random string
char *random_string(size_t length) {
   static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
   char             *str = malloc(length + 1);

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

// Function to generate a random JSON object
json_t *random_json_object(int depth, int max_depth) {
   if (depth > max_depth) {
      // Return a random terminal value
      int choice = rand() % 5;
      switch (choice) {
         case 0:
            return json_string(random_string(20));  // Random string
         case 1:
            return json_integer(rand() % 1000);  // Random number
         case 2:
            return json_true();  // Boolean true
         case 3:
            return json_false();  // Boolean false
         default:
            return json_null();  // Null value
      }
   }

   json_t *obj = json_object();
   if (!obj) {
      fprintf(stderr, "Failed to create JSON object.\n");
      exit(EXIT_FAILURE);
   }

   for (int i = 0; i < 5; i++) {  // Create 5 random key-value pairs
      char *key = random_string(10);

      int     choice = rand() % 5;
      json_t *value;
      switch (choice) {
         case 0:  // Random string
            value = json_string(random_string(20));
            break;
         case 1:  // Random number
            value = json_integer(rand() % 1000);
            break;
         case 2:  // Nested object
            value = random_json_object(depth + 1, max_depth);
            break;
         case 3: {  // Array of strings
            value = json_array();
            for (int j = 0; j < 5; j++) {
               json_array_append_new(value, json_string(random_string(20)));
            }
            break;
         }
         case 4: {  // Array of numbers
            value = json_array();
            for (int j = 0; j < 5; j++) {
               json_array_append_new(value, json_integer(rand() % 1000));
            }
            break;
         }
         default:
            value = json_null();
      }

      json_object_set_new(obj, key, value);
      free(key);  // Free the key after use
   }

   return obj;
}

// Function to write JSON incrementally to a file
void generate_random_json(const char *filename, int num_objects, int max_depth) {
   FILE *file = fopen(filename, "w");
   if (!file) {
      fprintf(stderr, "Failed to open file '%s' for writing.\n", filename);
      exit(EXIT_FAILURE);
   }

   srand((unsigned int)time(NULL));
   json_set_alloc_funcs(malloc, free);

   fprintf(file, "{\"root\": [");  // Start the JSON array

   for (int i = 0; i < num_objects; i++) {
      json_t *obj = random_json_object(1, max_depth);
      char   *json_str = json_dumps(obj, JSON_COMPACT);
      if (!json_str) {
         fprintf(stderr, "Failed to serialize JSON object.\n");
         exit(EXIT_FAILURE);
      }

      fprintf(file, "%s", json_str);
      free(json_str);  // Free serialized JSON string
      json_decref(obj);  // Decrease reference count and free object

      if (i < num_objects - 1) {
         fprintf(file, ",");  // Add a comma between objects
      }
   }

   fprintf(file, "]}");  // Close the JSON array
   fclose(file);

   printf("JSON successfully written to '%s'\n", filename);
}

typedef struct Bench {
   double mean, min, max, stddev;
} Bench;

void calc_bench(Bench *bench, clock_t *starts, clock_t *ends) {
   double durations[NUM_CYCLES];
   double sum = 0.0, mean, min, max, stddev = 0.0;

   // Convert clock ticks to seconds and store durations
   for (int i = 0; i < NUM_CYCLES; ++i) {
      durations[i] = (double)(ends[i] - starts[i]) / CLOCKS_PER_SEC;
      sum += durations[i];
   }

   // Calculate mean
   mean = sum / NUM_CYCLES;

   // Initialize min and max
   min = max = durations[0];
   for (int i = 1; i < NUM_CYCLES; ++i) {
      if (durations[i] < min)
         min = durations[i];
      if (durations[i] > max)
         max = durations[i];
   }

   // Calculate standard deviation
   for (int i = 0; i < NUM_CYCLES; ++i) {
      stddev += (durations[i] - mean) * (durations[i] - mean);
   }
   stddev = sqrt(stddev / NUM_CYCLES);  // or use (n - 1) for sample stddev

   bench->mean = mean;
   bench->min = min;
   bench->max = max;
   bench->stddev = stddev;
}

// Run tests `cycles` times
void benchmark(
   Bench      *bench,
   const char *filename,
   void (*init_func)(),
   void (*deinit_func)(),
   void *(*alloc_func)(size_t),
   void (*free_func)(void *)
) {
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

void print_bench(Bench *bench, const char *what) {
   printf("Results for `%s`: \n", what);
   printf("Average: %f seconds\n", bench->mean);
   printf("Minimum: %f seconds\n", bench->min);
   printf("Maximum: %f seconds\n", bench->max);
   printf("Std Dev: %f seconds\n", bench->stddev);
   printf("\n");
}

int main() {
   Bench       bench = {0};
   const char *filename = "benches\\test.json";

   if (!fopen(filename, "r"))
      remove(filename);
   generate_random_json(filename, 5000, 10);

   benchmark(&bench, filename, NULL, NULL, malloc, free);
   print_bench(&bench, "malloc");

   benchmark(&bench, filename, _arena_init, _arena_deinit, _arena_alloc, _arena_free);
   print_bench(&bench, "arena");

   return 0;
}