#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lookup3.h"

int main()
{
    const char* str = "Hello world.\n";
    uint32_t hash_val = hashlittle(str, strlen(str), 0x01010101);
    printf("%d\n", hash_val);
    return 0;
}