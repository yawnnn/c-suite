#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"

char *print_itos(void *pval, char *buf)
{
    if (buf == NULL)
        buf = (char *)malloc(12 + 1);
    sprintf(buf, "%d", *((int*)pval));

    return buf;
}

int main() 
{
    Vec v;

    int elem;
    printf("TESTING VECTOR OF INTEGERS...\n");

    vec_new(&v, sizeof(elem));

    elem = 2;
    vec_push(&v, &elem);
    elem = 4;
    vec_push(&v, &elem);
    elem = -1;
    vec_push(&v, &elem);

    vec_dbg(&v);
    vec_print(&v, print_itos);

    vec_drop(&v);

    /* TODO --- Dynamic string. For the moment there is onyl special case for vec_print */
    Vec v_str;
    char s1[] = "First string";
    char s2[] = "Second string";
    char s3[] = "Thrist and final string";

    printf("TESTING VECTOR OF STRINGS...\n");

    vec_new(&v, sizeof(Vec));

    vec_new_with(&v_str, sizeof(s1), sizeof(char));
    vec_push_n(&v_str, s1, sizeof(s1));
    vec_push(&v, &v_str);

    vec_new_with(&v_str, sizeof(s2), sizeof(char));
    vec_push_n(&v_str, s2, sizeof(s2));
    vec_push(&v, &v_str);

    vec_new_with(&v_str, sizeof(s3), sizeof(char));
    vec_push_n(&v_str, s3, sizeof(s3));
    vec_push(&v, &v_str);

    vec_print(&v, NULL);

    vec_iter_reset();
    while (vec_iter(&v, &v_str))
        vec_drop(&v_str);
    vec_drop(&v);
}