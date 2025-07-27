#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "ilist.h"

typedef struct {
   int   value;
   INode node;
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
   IList list;
   ilist_init(&list);
   assert(ilist_is_empty(&list));
   assert(ilist_first(&list) == NULL);
   assert(ilist_last(&list) == NULL);
   assert(ilist_next(&list, (INode *)&list) == NULL);
   assert(ilist_prev(&list, (INode *)&list) == NULL);
   printf("%s passed\n", __func__);
}

static void test_insert_and_order()
{
   IList list;
   ilist_init(&list);

   TestNode *n1 = make_node(1);
   TestNode *n2 = make_node(2);
   TestNode *n3 = make_node(3);

   ilist_push_back(&list, &n1->node);
   ilist_push_back(&list, &n2->node);
   ilist_push_front(&list, &n3->node);

   // list order: n3, n1, n2
   INode *curr = ilist_first(&list);
   assert(ilist_entry(curr, TestNode, node)->value == 3);
   curr = ilist_next(&list, curr);
   assert(ilist_entry(curr, TestNode, node)->value == 1);
   curr = ilist_next(&list, curr);
   assert(ilist_entry(curr, TestNode, node)->value == 2);
   curr = ilist_next(&list, curr);
   assert(curr == NULL);

   free(n1);
   free(n2);
   free(n3);

   printf("%s passed\n", __func__);
}

static void test_remove()
{
   IList list;
   ilist_init(&list);

   TestNode *n1 = make_node(10);
   TestNode *n2 = make_node(20);

   ilist_push_back(&list, &n1->node);
   ilist_push_back(&list, &n2->node);
   assert(!ilist_is_empty(&list));

   ilist_remove(&n1->node);
   assert(ilist_entry(ilist_first(&list), TestNode, node)->value == 20);

   ilist_remove(&n2->node);
   assert(ilist_is_empty(&list));

   free(n1);
   free(n2);

   printf("%s passed\n", __func__);
}

static void test_pop()
{
   IList list;
   ilist_init(&list);

   TestNode *n1 = make_node(100);
   TestNode *n2 = make_node(200);

   ilist_push_back(&list, &n1->node);
   ilist_push_back(&list, &n2->node);

   ilist_pop_back(&list);  // remove n2
   assert(ilist_entry(ilist_first(&list), TestNode, node)->value == 100);

   ilist_pop_back(&list);  // remove n1
   assert(ilist_is_empty(&list));

   free(n1);
   free(n2);

   printf("%s passed\n", __func__);
}

static void test_insert_and_forward_iteration()
{
   IList list;
   ilist_init(&list);

   TestNode *n1 = make_node(1);
   TestNode *n2 = make_node(2);
   TestNode *n3 = make_node(3);

   ilist_push_back(&list, &n1->node);
   ilist_push_back(&list, &n2->node);
   ilist_push_back(&list, &n3->node);

   int expected[] = {1, 2, 3};
   int idx = 0;
   for (INode *curr = ilist_first(&list); curr != NULL; curr = ilist_next(&list, curr)) {
      TestNode *t = ilist_entry(curr, TestNode, node);
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
   IList list;
   ilist_init(&list);

   TestNode *n1 = make_node(10);
   TestNode *n2 = make_node(20);
   TestNode *n3 = make_node(30);

   ilist_push_back(&list, &n1->node);
   ilist_push_back(&list, &n2->node);
   ilist_push_back(&list, &n3->node);

   // Start from last
   INode *curr = ilist_last(&list);
   int    expected[] = {30, 20, 10};
   int    idx = 0;

   while (curr != NULL) {
      TestNode *t = ilist_entry(curr, TestNode, node);
      assert(t->value == expected[idx]);
      idx++;
      curr = ilist_prev(&list, curr);
   }
   assert(idx == 3);

   free(n1);
   free(n2);
   free(n3);

   printf("%s passed\n", __func__);
}

static void test_forward_backward_symmetry()
{
   IList list;
   ilist_init(&list);

   // Insert nodes
   for (int i = 0; i < NTESTS; i++) {
      TestNode *n = make_node(i + 1);
      ilist_push_back(&list, &n->node);
   }

   // Collect forward
   int fwd[NTESTS], bwd[NTESTS], i = 0;
   // this for pattern is also valid
   for (INode *curr = (INode *)&list; (curr = ilist_next(&list, curr)) != NULL;) {
      fwd[i++] = ilist_entry(curr, TestNode, node)->value;
   }
   assert(i == NTESTS);

   // Collect backward
   i = 0;
   for (INode *curr = ilist_last(&list); curr != NULL; curr = ilist_prev(&list, curr)) {
      TestNode *entry = ilist_entry(curr, TestNode, node);
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
   IList list;
   ilist_init(&list);

   TestNode *n1 = make_node(1);
   ilist_push_back(&list, &n1->node);

   assert(!ilist_is_empty(&list));
   assert(ilist_first(&list) == &n1->node);
   assert(ilist_last(&list) == &n1->node);

   // next/prev of the only element should be NULL
   assert(ilist_next(&list, &n1->node) == NULL);
   assert(ilist_prev(&list, &n1->node) == NULL);

   ilist_remove(&n1->node);
   free(n1);
   assert(ilist_is_empty(&list));

   printf("%s passed\n", __func__);
}

static void test_forward_backward_iteration()
{
   IList list;
   ilist_init(&list);

   TestNode *nodes[NTESTS];
   for (int i = 0; i < NTESTS; i++) {
      nodes[i] = make_node(i + 1);
      ilist_push_back(&list, &nodes[i]->node);
   }

   // Forward iteration
   INode *curr = ilist_first(&list);
   int    expected = 0;
   while (curr) {
      TestNode *tn = ilist_entry(curr, TestNode, node);
      assert(tn->value == ++expected);
      curr = ilist_next(&list, curr);
   }
   assert(expected == NTESTS);

   // Backward iteration
   curr = ilist_last(&list);
   expected = NTESTS;
   while (curr) {
      TestNode *tn = ilist_entry(curr, TestNode, node);
      assert(tn->value == expected--);
      curr = ilist_prev(&list, curr);
   }
   assert(expected == 0);

   for (int i = 0; i < NTESTS; i++)
      free(nodes[i]);

   printf("%s passed\n", __func__);
}

static void test_mixed_insert_remove()
{
   IList list;
   ilist_init(&list);

   TestNode *a = make_node(1);
   TestNode *b = make_node(2);
   TestNode *c = make_node(3);

   ilist_push_front(&list, &b->node);
   ilist_push_front(&list, &a->node);
   ilist_push_back(&list, &c->node);
   // list: a,b,c

   assert(ilist_entry(ilist_first(&list), TestNode, node)->value == 1);
   assert(ilist_entry(ilist_last(&list), TestNode, node)->value == 3);

   ilist_remove(&b->node);
   // list: a,c
   assert(ilist_entry(ilist_first(&list), TestNode, node)->value == 1);
   assert(ilist_entry(ilist_last(&list), TestNode, node)->value == 3);

   ilist_remove(&a->node);
   ilist_remove(&c->node);
   assert(ilist_is_empty(&list));

   free(a);
   free(b);
   free(c);

   printf("%s passed\n", __func__);
}

static void test_split_front_and_back()
{
   IList list, dst;
   ilist_init(&list);
   ilist_init(&dst);

   TestNode *nodes[NTESTS];
   for (int i = 0; i < NTESTS; i++) {
      nodes[i] = make_node(i + 1);
      ilist_push_back(&list, &nodes[i]->node);
   }
   // list: 1,2,3,4,5

   // Split after node "2": dst gets 3,4,5; list keeps 1,2
   INode *at = ilist_get(&list, 1);  // index 1 => node "2"
   ilist_split_front(&dst, &list, at);

   // Verify list: 1,2
   int expected1[] = {1, 2};
   int idx = 0;
   for (INode *curr = ilist_first(&list); curr; curr = ilist_next(&list, curr)) {
      TestNode *tn = ilist_entry(curr, TestNode, node);
      assert(tn->value == expected1[idx++]);
   }
   assert(idx == 2);

   // Verify dst: 3,4,5
   int expected2[] = {3, 4, 5};
   idx = 0;
   for (INode *curr = ilist_first(&dst); curr; curr = ilist_next(&dst, curr)) {
      TestNode *tn = ilist_entry(curr, TestNode, node);
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
   IList list, dst;
   ilist_init(&list);
   ilist_init(&dst);

   TestNode *nodes[NTESTS];
   for (int i = 0; i < NTESTS; i++) {
      nodes[i] = make_node(i + 1);
      ilist_push_back(&list, &nodes[i]->node);
   }
   // list: 1,2,3,4,5

   // Split until before node "4": dst gets 1,2,3; list keeps 4,5
   INode *at = ilist_get(&list, 3);  // index 3 => node "4"
   ilist_split_back(&dst, &list, at);

   // Verify dst: 1,2,3
   int expected1[] = {1, 2, 3};
   int idx = 0;
   for (INode *curr = ilist_first(&dst); curr; curr = ilist_next(&dst, curr)) {
      TestNode *tn = ilist_entry(curr, TestNode, node);
      assert(tn->value == expected1[idx++]);
   }
   assert(idx == 3);

   // Verify list: 4,5
   int expected2[] = {4, 5};
   idx = 0;
   for (INode *curr = ilist_first(&list); curr; curr = ilist_next(&list, curr)) {
      TestNode *tn = ilist_entry(curr, TestNode, node);
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
   IList list1, list2;
   ilist_init(&list1);
   ilist_init(&list2);

   TestNode *a = make_node(1);
   TestNode *b = make_node(2);
   TestNode *c = make_node(3);
   TestNode *d = make_node(4);

   ilist_push_back(&list1, &a->node);
   ilist_push_back(&list1, &b->node);

   ilist_push_back(&list2, &c->node);
   ilist_push_back(&list2, &d->node);

   // Insert list2 after node "a": list1 should become 1,c,d,2
   ilist_join_front(&list2, &a->node);

   int expected[] = {1, 3, 4, 2};
   int idx = 0;
   for (INode *curr = ilist_first(&list1); curr; curr = ilist_next(&list1, curr)) {
      TestNode *tn = ilist_entry(curr, TestNode, node);
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
   IList list;
   ilist_init(&list);

   TestNode *n1 = make_node(1);
   TestNode *n2 = make_node(2);
   TestNode *n3 = make_node(3);

   ilist_push_back(&list, &n1->node);
   ilist_push_back(&list, &n2->node);
   ilist_push_back(&list, &n3->node);

   // Replace n1 with n3: list becomes 3,2,1
   ilist_swap(&n1->node, &n3->node);
   assert(ilist_entry(ilist_first(&list), TestNode, node)->value == 3);

   // list becomes 3,2
   ilist_remove(&n1->node);

   // Swap 3 and 2: list becomes 2,3
   INode *first = ilist_first(&list);
   INode *second = ilist_next(&list, first);
   ilist_swap(first, second);

   assert(ilist_entry(ilist_first(&list), TestNode, node)->value == 2);
   assert(ilist_entry(ilist_last(&list), TestNode, node)->value == 3);

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
