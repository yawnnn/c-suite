#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "vstr.h"

void test_vstr_new()
{
   Vstr vs;
   vstr_new(&vs);
   assert(vstr_data(&vs) == NULL);
   assert(vstr_is_empty(&vs));
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_new_with()
{
   Vstr vs;
   vstr_new_with(&vs, 32);
   assert(vstr_data(&vs) != NULL);
   assert(vstr_is_empty(&vs));
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_from()
{
   Vstr vs;
   vstr_from(&vs, "hello");
   assert(strcmp(vstr_data(&vs), "hello") == 0);
   assert(!vstr_is_empty(&vs));
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_truncate()
{
   Vstr vs;
   vstr_from(&vs, "abcdef");
   vstr_truncate(&vs, 3);
   assert(strcmp(vstr_data(&vs), "abc") == 0);
   vstr_truncate(&vs, 10);  // Should have no effect
   assert(strcmp(vstr_data(&vs), "abc") == 0);
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_reserve_and_shrink()
{
   Vstr vs;
   vstr_new(&vs);
   vstr_reserve(&vs, 64);
   assert(vstr_data(&vs) != NULL);
   vstr_cpy(&vs, "short");
   vstr_shrink_to_fit(&vs);
   assert(strcmp(vstr_data(&vs), "short") == 0);
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_insert()
{
   Vstr vs;
   vstr_from(&vs, "world");
   vstr_insert(&vs, 0, "hello ", 6);  // "hello world"
   assert(strcmp(vstr_data(&vs), "hello world") == 0);
   vstr_insert(&vs, 100, "!", 1);  // Should be ignored (past end)
   assert(strcmp(vstr_data(&vs), "hello world") == 0);
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_cpy_ncpy()
{
   Vstr vs;
   vstr_new_with(&vs, 32);
   vstr_cpy(&vs, "foobar");
   assert(strcmp(vstr_data(&vs), "foobar") == 0);
   vstr_ncpy(&vs, "abcdefg", 3);
   assert(strncmp(vstr_data(&vs), "abc", 3) == 0);
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_cat_ncat()
{
   Vstr vs;
   vstr_from(&vs, "foo");
   vstr_cat(&vs, "bar");
   assert(strcmp(vstr_data(&vs), "foobar") == 0);
   vstr_ncat(&vs, "bazqux", 3);
   assert(strcmp(vstr_data(&vs), "foobarbaz") == 0);
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_data_from()
{
   Vstr vs;
   vstr_from(&vs, "example");
   char *sub = vstr_at(&vs, 3);
   assert(strcmp(sub, "mple") == 0);
   assert(vstr_at(&vs, 100) == NULL);
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_is_empty()
{
   Vstr vs;
   vstr_new(&vs);
   assert(vstr_is_empty(&vs));
   vstr_cpy(&vs, "nonempty");
   assert(!vstr_is_empty(&vs));
   vstr_truncate(&vs, 0);
   assert(vstr_is_empty(&vs));
   vstr_free(&vs);
   
   printf("%s passed\n", __func__);
}

void test_vstr_sprintf()
{
   Vstr  vs = {0};
   int   n = vstr_sprintf(&vs, 0, "This is %s n.%03d", "test", 1);
   char *res1 = "This is test n.001";
   assert(!strcmp(vstr_data(&vs), res1));
   assert(n == strlen(res1));
   int offset = strlen(res1) - 3;
   n = vstr_sprintf(&vs, offset, "%04d", 2);
   char *res2 = "This is test n.0002";
   assert(!strcmp(vstr_data(&vs), res2));
   assert(n == strlen(res2) - offset);
   
   printf("%s passed\n", __func__);
}

int main()
{
   test_vstr_new();
   test_vstr_new_with();
   test_vstr_from();
   test_vstr_truncate();
   test_vstr_reserve_and_shrink();
   test_vstr_insert();
   test_vstr_cpy_ncpy();
   test_vstr_cat_ncat();
   test_vstr_data_from();
   test_vstr_is_empty();
   test_vstr_sprintf();

   printf("%s suite passed!\n", __FILE__);
   return 0;
}
