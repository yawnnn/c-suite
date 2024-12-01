#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arena_allocator.h"

#include "jansson.h"

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
json_t *generate_object(int depth, int max_depth) {
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
            value = generate_object(depth + 1, max_depth);
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
void write_large_json(const char *filename, int num_objects, int max_depth) {
   FILE *file = fopen(filename, "w");
   if (!file) {
      fprintf(stderr, "Failed to open file '%s' for writing.\n", filename);
      exit(EXIT_FAILURE);
   }

   fprintf(file, "{\"root\": [");  // Start the JSON array

   for (int i = 0; i < num_objects; i++) {
      json_t *obj = generate_object(1, max_depth);
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

int make_test_json(const char *filename, int num_objects, int max_depth) {
   if (num_objects <= 0 || max_depth <= 0) {
      fprintf(stderr, "Error: num_objects and max_depth must be positive integers.\n");
      return EXIT_FAILURE;
   }

   srand((unsigned int)time(NULL));  // Seed the random number generator

   // Generate and write JSON file
   write_large_json(filename, num_objects, max_depth);

   return EXIT_SUCCESS;
}

static ArenaAllocator my = {0};

void my_init() {
   arena_allocator_init(&my, 4096 * 100);
}

void my_deinit() {
   arena_allocator_deinit(&my);
}

void *my_malloc(size_t size) {
   return arena_allocator_alloc(&my, size);
}

void my_free(void *ptr) {
   arena_allocator_free(&my, ptr);
}

// Function to run a single test 100 times with the specified allocator
double run_test(
   const char *filename,
   int         runs,
   void (*init_func)(),
   void (*deinit_func)(),
   void *(*alloc_func)(size_t),
   void (*free_func)(void *)
) {
   json_set_alloc_funcs(alloc_func, free_func);

   clock_t start_time = clock();

   for (int i = 0; i < runs; i++) {
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
   }

   clock_t end_time = clock();

   return (double)(end_time - start_time) / CLOCKS_PER_SEC;
}

int main() {
   const char *filename = "benches\\test.json";
   int         runs = 5;

   if (!fopen(filename, "r"))
      make_test_json(filename, 5000, 10);

   // Test with standard malloc/free
   printf("\nTesting with standard malloc/free:\n");
   double standard_time = run_test(filename, runs, NULL, NULL, malloc, free);
   printf("Standard allocator time for %d runs: %.6f seconds\n", runs, standard_time);

   // Test with my allocator
   printf("Testing with my allocator:\n");
   double my_time = run_test(filename, runs, my_init, my_deinit, my_malloc, my_free);
   printf("my allocator time for %d runs: %.6f seconds\n", runs, my_time);

   // Display comparison
   printf("\nComparison:\n");
   printf("Standard allocator total time: %.6f seconds\n", standard_time);
   printf("my allocator total time: %.6f seconds\n", my_time);

   return 0;
}