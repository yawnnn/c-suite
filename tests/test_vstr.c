#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vstr.h"

int main() {
    Vstr s;
    
    vstr_new(&s);
    vstr_ncpy(&s, "first", 4);
    vstr_ncat(&s, "second", 4);
    vstr_ncat(&s, "third", 6);
    
    printf("%s", vstr_data(&s));
    
    vstr_free(&s);
}