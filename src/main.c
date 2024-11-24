#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "arena_allocator.h"
#include "fixed_allocator.h"
#include "hashmap.h"
#include "jansson.h"
#include "llist.h"
#include "tracking_allocator.h"
#include "vec.h"
#include "vstr.h"

static ArenaAllocator allocator = {0};

void *_malloc(size_t size) {
   return arena_allocator_alloc(&allocator, size);
}

void _free(void *ptr) {
   arena_allocator_free(&allocator, ptr);
}

void init() {
   arena_allocator_init(&allocator, 4096);
}

void deinit() {
   arena_allocator_deinit(&allocator);
}

// Function to run a single test 100 times with the specified allocator
double run_test(
   const char *filename,
   void (*init_func)(),
   void (*deinit_func)(),
   void *(*alloc_func)(size_t),
   void (*free_func)(void *)
) {
   json_set_alloc_funcs(alloc_func, free_func);  // Set the allocation functions

   clock_t start_time = clock();  // Start the clock

   if (init_func)
      init_func();  // Initialize the allocator or other setup

   for (int i = 0; i < 100; i++) {
      json_error_t error;
      json_t      *root = json_load_file(filename, 0, &error);  // Load the JSON file
      
      if (!root) {
         printf("Failed to load JSON file on iteration %d: %s\n", i + 1, error.text);
      }
      else {
         json_decref(root);  // Free JSON object after use
      }
   }

   if (deinit_func)
      deinit_func();  // Clean up the allocator or other teardown

   clock_t end_time = clock();  // End the clock

   return (double)(end_time - start_time) / CLOCKS_PER_SEC;  // Return elapsed time
}

int main() {
   const char *filename = "large_json.json";

   // Test with custom allocator
   printf("Testing with custom allocator:\n");
   double custom_time = run_test(filename, init, deinit, _malloc, _free);
   printf("Custom allocator time for 100 runs: %.6f seconds\n", custom_time);

   // Test with standard malloc/free
   printf("\nTesting with standard malloc/free:\n");
   double standard_time = run_test(filename, NULL, NULL, malloc, free);
   printf("Standard allocator time for 100 runs: %.6f seconds\n", standard_time);

   // Display comparison
   printf("\nComparison:\n");
   printf("Custom allocator total time: %.6f seconds\n", custom_time);
   printf("Standard allocator total time: %.6f seconds\n", standard_time);

   return 0;
}

//             //
// Linked list //
//             //
// void dbg_front(LList *list) {
//     printf("\n### FORWARDS ###\n");

//     bool first = true;
//     bool found_tail = false;
//     for (LLNode *next = NULL; (next = llist_next(list, next)) != NULL;) {
//         printf("%s%d", first ? "" : " -> ", *(int *)next->data);
//         first = false;

//         if (next == list->tail)
//             found_tail = true;
//     }
//     if (!first)
//         printf("\n");
//     printf("found_tail: %d\n", (int)found_tail);
// }

// void dbg_back(LList *list) {
//     printf("\n### BACKWARDS ###\n");

//     bool first = true;
//     bool found_head = false;
//     for (LLNode *next = NULL; (next = llist_prev(list, next)) != NULL;) {
//         printf("%s%d", first ? "" : " -> ", *(int *)next->data);
//         first = false;

//         if (next == list->tail)
//             found_head = true;
//     }
//     if (!first)
//         printf("\n");
//     printf("found_head: %d\n", (int)found_head);
// }

// int *get_unique_id() {
//     static int s_global_counter = 0;
//     int *id = (int *)malloc(sizeof(int));
//     *id = ++s_global_counter;
//     return id;
// }

// int main() {
//     LList list = {0};

//     LLNode *n1 = llist_push_back(&list, get_unique_id());      int *v1;
//     LLNode *n2 = llist_push_back(&list, get_unique_id());      int *v2;
//     LLNode *n3 = llist_push_front(&list, get_unique_id());     int *v3;
//     LLNode *n4 = llist_push_front(&list, get_unique_id());     int *v4;
//     LLNode *n5 = llist_push_back(&list, get_unique_id());      int *v5;

//     dbg_front(&list);//       dbg_back(&list);

//     v4 = llist_pop_front(&list);      free(v4);
//     v5 = llist_pop_back(&list);       free(v5);

//     dbg_front(&list);//       dbg_back(&list);

//     LLNode *n6 = llist_insert(&list, llist_prev(&list, n2), get_unique_id());     int *v6;
//     LLNode *n7 = llist_insert(&list, llist_next(&list, n3), get_unique_id());     int *v7;
//     LLNode *n8 = llist_insert(&list, llist_prev(&list, n1), get_unique_id());     int *v8;

//     dbg_front(&list);       dbg_back(&list);

//     v1 = llist_remove(&list, n1);      free(v1);
//     v7 = llist_remove(&list, n7);      free(v7);
//     v6 = llist_remove(&list, llist_prev(&list, n2));      free(v6);
//     v8 = llist_remove(&list, llist_prev(&list, n2));      free(v8);

//     LLNode *n9 = llist_insert(&list, llist_prev(&list, n2), get_unique_id());     int *v9;

//     v2 = llist_remove(&list, list.tail);      free(v2);
//     v3 = llist_remove(&list, list.head);      free(v3);
//     //v9 = llist_remove(&list, list.head);      free(v9);
//     v9 = llist_remove(&list, list.tail);      free(v9);
//     //v__ = llist_remove(&list, n__);      free(v__);

//     // v3 = llist_remove(&list, n3);      free(v3);
//     // v8 = llist_remove(&list, n8);      free(v8);
//     // v1 = llist_remove(&list, n1);      free(v1);
//     // v7 = llist_remove(&list, n7);      free(v7);
//     // v6 = llist_remove(&list, n6);      free(v6);
//     // v2 = llist_remove(&list, n2);      free(v2);
// }

//             //
// vstr        //
//             //
// int main() {
//     Vstr s;

//     vstr_new(&s);
//     vstr_ncpy(&s, "ciao sono una mamma", 4);
//     vstr_ncat(&s, "ciao sono una mamma", 4);
//     vstr_ncat(&s, "ciao sono una mamma", 6);

//     printf(s.ptr);

//     vstr_free(&s);

//     printf("\nDone");
// }

// void debug(HashMap *hm) {
//    printf("HASHMAP:\n");
//    for (size_t i = 0; i < hm->count; i++) {
//        Node *node = hashmap_get(hm, i);

//        if (node)
//            printf("key: %d, value: %d\n", node->key, node->value);
//    }
//    printf("\n");
// }

// int main() {
//    HashMap hm;
//    Value val = 0;

//    hashmap_new(&hm);
//    hashmap_put(&hm, 0, val++);
//    hashmap_put(&hm, 0, val++);
//    hashmap_put(&hm, 1, val++);
//    hashmap_remove(&hm, 0);
//    debug(&hm);
//    hashmap_put(&hm, 0, val++);
//    hashmap_remove(&hm, 0);
//    hashmap_put(&hm, 2, val++);
//    hashmap_put(&hm, 1, val++);
//    debug(&hm);

//    hashmap_free(&hm);

//    printf("DONE");
// }