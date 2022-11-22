#include <stddef.h>

size_t hash(char *key)
{
    size_t i = 0;
    size_t hash = 0;

    for (i = 0; key[i] != '\0'; ++i)
        hash += 'A' <= key[i] && key[i] <= 'Z' ? key[i] - 'A' + 'a' : key[i];
    hash += i;

    return hash;
}
