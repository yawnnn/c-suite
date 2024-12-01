#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vstr.h"

int main() {
    Vstr s;

    vstr_new(&s);
    vstr_ncpy(&s, "hello world", 4);
    vstr_ncat(&s, "hello world", 4);
    vstr_ncat(&s, "hello world", 6);

    printf(vstr_data(&s));

    vstr_free(&s);
}