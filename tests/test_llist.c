#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "llist.h"

typedef struct {
   int   value;
   LNode node;
} TestNode;

#define NTESTS 5

static TestNode *make_node(int v)
{
   TestNode *n = calloc(1, sizeof(TestNode));
   assert(n);
   n->value = v;
   return n;
}

static void test_init_empty()
{
   LList list;
   llist_init(&list);
   assert(llist_is_empty(&list));
   assert(llist_first(&list) == NULL);
   assert(llist_last(&list) == NULL);
   assert(llist_next(&list, (LNode *)&list) == NULL);
   assert(llist_prev(&list, (LNode *)&list) == NULL);
   printf("%s passed\n", __func__);
}

static void test_insert_and_order()
{
   LList list;
   llist_init(&list);

   TestNode *n1 = make_node(1);
   TestNode *n2 = make_node(2);
   TestNode *n3 = make_node(3);

   llist_push_back(&list, &n1->node);
   llist_push_back(&list, &n2->node);
   llist_push_front(&list, &n3->node);

   // list order: n3, n1, n2
   LNode *curr = llist_first(&list);
   assert(llist_entry(curr, TestNode, node)->value == 3);
   curr = llist_next(&list, curr);
   assert(llist_entry(curr, TestNode, node)->value == 1);
   curr = llist_next(&list, curr);
   assert(llist_entry(curr, TestNode, node)->value == 2);
   curr = llist_next(&list, curr);
   assert(curr == NULL);

   free(n1);
   free(n2);
   free(n3);

   printf("%s passed\n", __func__);
}

static void test_remove()
{
   LList list;
   llist_init(&list);

   TestNode *n1 = make_node(10);
   TestNode *n2 = make_node(20);

   llist_push_back(&list, &n1->node);
   llist_push_back(&list, &n2->node);
   assert(!llist_is_empty(&list));

   llist_remove(&n1->node);
   assert(llist_entry(llist_first(&list), TestNode, node)->value == 20);

   llist_remove(&n2->node);
   assert(llist_is_empty(&list));

   free(n1);
   free(n2);

   printf("%s passed\n", __func__);
}

static void test_pop()
{
   LList list;
   llist_init(&list);

   TestNode *n1 = make_node(100);
   TestNode *n2 = make_node(200);

   llist_push_back(&list, &n1->node);
   llist_push_back(&list, &n2->node);

   llist_pop_back(&list);  // remove n2
   assert(llist_entry(llist_first(&list), TestNode, node)->value == 100);

   llist_pop_back(&list);  // remove n1
   assert(llist_is_empty(&list));

   free(n1);
   free(n2);

   printf("%s passed\n", __func__);
}

static void test_insert_and_forward_iteration()
{
   LList list;
   llist_init(&list);

   TestNode *n1 = make_node(1);
   TestNode *n2 = make_node(2);
   TestNode *n3 = make_node(3);

   llist_push_back(&list, &n1->node);
   llist_push_back(&list, &n2->node);
   llist_push_back(&list, &n3->node);

   int expected[] = {1, 2, 3};
   int idx = 0;
   for (LNode *curr = llist_first(&list); curr != NULL; curr = llist_next(&list, curr)) {
      TestNode *t = llist_entry(curr, TestNode, node);
      assert(t->value == expected[idx]);
      idx++;
   }
   assert(idx == 3);

   free(n1);
   free(n2);
   free(n3);

   printf("%s passed\n", __func__);
}

static void test_backward_iteration()
{
   LList list;
   llist_init(&list);

   TestNode *n1 = make_node(10);
   TestNode *n2 = make_node(20);
   TestNode *n3 = make_node(30);

   llist_push_back(&list, &n1->node);
   llist_push_back(&list, &n2->node);
   llist_push_back(&list, &n3->node);

   // Start from last
   LNode *curr = llist_last(&list);
   int    expected[] = {30, 20, 10};
   int    idx = 0;

   while (curr != NULL) {
      TestNode *t = llist_entry(curr, TestNode, node);
      assert(t->value == expected[idx]);
      idx++;
      curr = llist_prev(&list, curr);
   }
   assert(idx == 3);

   free(n1);
   free(n2);
   free(n3);

   printf("%s passed\n", __func__);
}

static void test_forward_backward_symmetry()
{
   LList list;
   llist_init(&list);

   // Insert nodes
   for (int i = 0; i < NTESTS; i++) {
      TestNode *n = make_node(i + 1);
      llist_push_back(&list, &n->node);
   }

   // Collect forward
   int fwd[NTESTS], bwd[NTESTS], i = 0;
   // this for pattern is also valid
   for (LNode *curr = (LNode *)&list; (curr = llist_next(&list, curr)) != NULL;) {
      fwd[i++] = llist_entry(curr, TestNode, node)->value;
   }
   assert(i == NTESTS);

   // Collect backward
   i = 0;
   for (LNode *curr = llist_last(&list); curr != NULL; curr = llist_prev(&list, curr)) {
      TestNode *entry = llist_entry(curr, TestNode, node);
      bwd[i++] = entry->value;
      free(entry);
   }
   assert(i == NTESTS);

   // Check symmetry
   for (int j = 0; j < NTESTS; j++) {
      assert(fwd[j] == (bwd[NTESTS - 1 - j]));
   }
   printf("%s passed\n", __func__);
}

static void test_single_element_list()
{
   LList list;
   llist_init(&list);

   TestNode *n1 = make_node(1);
   llist_push_back(&list, &n1->node);

   assert(!llist_is_empty(&list));
   assert(llist_first(&list) == &n1->node);
   assert(llist_last(&list) == &n1->node);

   // next/prev of the only element should be NULL
   assert(llist_next(&list, &n1->node) == NULL);
   assert(llist_prev(&list, &n1->node) == NULL);

   llist_remove(&n1->node);
   free(n1);
   assert(llist_is_empty(&list));

   printf("%s passed\n", __func__);
}

static void test_forward_backward_iteration()
{
   LList list;
   llist_init(&list);

   TestNode *nodes[NTESTS];
   for (int i = 0; i < NTESTS; i++) {
      nodes[i] = make_node(i + 1);
      llist_push_back(&list, &nodes[i]->node);
   }

   // Forward iteration
   LNode *curr = llist_first(&list);
   int    expected = 0;
   while (curr) {
      TestNode *tn = llist_entry(curr, TestNode, node);
      assert(tn->value == ++expected);
      curr = llist_next(&list, curr);
   }
   assert(expected == NTESTS);

   // Backward iteration
   curr = llist_last(&list);
   expected = NTESTS;
   while (curr) {
      TestNode *tn = llist_entry(curr, TestNode, node);
      assert(tn->value == expected--);
      curr = llist_prev(&list, curr);
   }
   assert(expected == 0);

   for (int i = 0; i < NTESTS; i++)
      free(nodes[i]);

   printf("%s passed\n", __func__);
}

static void test_mixed_insert_remove()
{
   LList list;
   llist_init(&list);

   TestNode *a = make_node(1);
   TestNode *b = make_node(2);
   TestNode *c = make_node(3);

   llist_push_front(&list, &b->node);
   llist_push_front(&list, &a->node);
   llist_push_back(&list, &c->node);
   // list: a,b,c

   assert(llist_entry(llist_first(&list), TestNode, node)->value == 1);
   assert(llist_entry(llist_last(&list), TestNode, node)->value == 3);

   llist_remove(&b->node);
   // list: a,c
   assert(llist_entry(llist_first(&list), TestNode, node)->value == 1);
   assert(llist_entry(llist_last(&list), TestNode, node)->value == 3);

   llist_remove(&a->node);
   llist_remove(&c->node);
   assert(llist_is_empty(&list));

   free(a);
   free(b);
   free(c);

   printf("%s passed\n", __func__);
}

static void test_split_front_and_back()
{
   LList list, dst;
   llist_init(&list);
   llist_init(&dst);

   TestNode *nodes[NTESTS];
   for (int i = 0; i < NTESTS; i++) {
      nodes[i] = make_node(i + 1);
      llist_push_back(&list, &nodes[i]->node);
   }
   // list: 1,2,3,4,5

   // Split after node "2": dst gets 3,4,5; list keeps 1,2
   LNode *at = llist_get(&list, 1);  // index 1 => node "2"
   llist_split_front(&dst, &list, at);

   // Verify list: 1,2
   int expected1[] = {1, 2};
   int idx = 0;
   for (LNode *curr = llist_first(&list); curr; curr = llist_next(&list, curr)) {
      TestNode *tn = llist_entry(curr, TestNode, node);
      assert(tn->value == expected1[idx++]);
   }
   assert(idx == 2);

   // Verify dst: 3,4,5
   int expected2[] = {3, 4, 5};
   idx = 0;
   for (LNode *curr = llist_first(&dst); curr; curr = llist_next(&dst, curr)) {
      TestNode *tn = llist_entry(curr, TestNode, node);
      assert(tn->value == expected2[idx++]);
   }
   assert(idx == 3);

   // Cleanup
   for (int i = 0; i < NTESTS; i++)
      free(nodes[i]);

   printf("%s passed\n", __func__);
}

static void test_split_back()
{
   LList list, dst;
   llist_init(&list);
   llist_init(&dst);

   TestNode *nodes[NTESTS];
   for (int i = 0; i < NTESTS; i++) {
      nodes[i] = make_node(i + 1);
      llist_push_back(&list, &nodes[i]->node);
   }
   // list: 1,2,3,4,5

   // Split until before node "4": dst gets 1,2,3; list keeps 4,5
   LNode *at = llist_get(&list, 3);  // index 3 => node "4"
   llist_split_back(&dst, &list, at);

   // Verify dst: 1,2,3
   int expected1[] = {1, 2, 3};
   int idx = 0;
   for (LNode *curr = llist_first(&dst); curr; curr = llist_next(&dst, curr)) {
      TestNode *tn = llist_entry(curr, TestNode, node);
      assert(tn->value == expected1[idx++]);
   }
   assert(idx == 3);

   // Verify list: 4,5
   int expected2[] = {4, 5};
   idx = 0;
   for (LNode *curr = llist_first(&list); curr; curr = llist_next(&list, curr)) {
      TestNode *tn = llist_entry(curr, TestNode, node);
      assert(tn->value == expected2[idx++]);
   }
   assert(idx == 2);

   // Cleanup
   for (int i = 0; i < NTESTS; i++)
      free(nodes[i]);

   printf("%s passed\n", __func__);
}

static void test_join_front_and_back()
{
   LList list1, list2;
   llist_init(&list1);
   llist_init(&list2);

   TestNode *a = make_node(1);
   TestNode *b = make_node(2);
   TestNode *c = make_node(3);
   TestNode *d = make_node(4);

   llist_push_back(&list1, &a->node);
   llist_push_back(&list1, &b->node);

   llist_push_back(&list2, &c->node);
   llist_push_back(&list2, &d->node);

   // Insert list2 after node "a": list1 should become 1,c,d,2
   llist_join_front(&list2, &a->node);

   int expected[] = {1, 3, 4, 2};
   int idx = 0;
   for (LNode *curr = llist_first(&list1); curr; curr = llist_next(&list1, curr)) {
      TestNode *tn = llist_entry(curr, TestNode, node);
      assert(tn->value == expected[idx++]);
   }
   assert(idx == 4);

   // Cleanup
   free(a);
   free(b);
   free(c);
   free(d);

   printf("%s passed\n", __func__);
}

static void test_replace_and_swap()
{
   LList list;
   llist_init(&list);

   TestNode *n1 = make_node(1);
   TestNode *n2 = make_node(2);
   TestNode *n3 = make_node(3);

   llist_push_back(&list, &n1->node);
   llist_push_back(&list, &n2->node);
   llist_push_back(&list, &n3->node);

   // Replace n1 with n3: list becomes 3,2,1
   llist_swap(&n1->node, &n3->node);
   assert(llist_entry(llist_first(&list), TestNode, node)->value == 3);

   // list becomes 3,2
   llist_remove(&n1->node);

   // Swap 3 and 2: list becomes 2,3
   LNode *first = llist_first(&list);
   LNode *second = llist_next(&list, first);
   llist_swap(first, second);

   assert(llist_entry(llist_first(&list), TestNode, node)->value == 2);
   assert(llist_entry(llist_last(&list), TestNode, node)->value == 3);

   free(n1);
   free(n2);
   free(n3);

   printf("%s passed\n", __func__);
}

int main()
{
   test_init_empty();
   test_insert_and_order();
   test_remove();
   test_pop();
   test_insert_and_forward_iteration();
   test_backward_iteration();
   test_forward_backward_symmetry();
   test_single_element_list();
   test_forward_backward_iteration();
   test_mixed_insert_remove();
   test_split_front_and_back();
   test_split_back();
   test_join_front_and_back();
   test_replace_and_swap();
   printf("%s suite passed!\n", __FILE__);
   return 0;
}
