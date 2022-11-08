#include <stdio.h>

#include "hash_map.h"

void hash_map_dump(struct hash_map *hash_map, const char *prefix)
{
    if (hash_map == NULL)
        return;

    for (size_t i = 0; i < hash_map->size; ++i)
    {
        struct pair_list *list = hash_map->data[i];
        if (list == NULL)
            continue;
        printf("%s%s: %s", prefix == NULL ? "" : prefix, list->key,
               list->value);
        for (list = list->next; list != NULL; list = list->next)
            printf("\n%s%s: %s", prefix, list->key, list->value);
        putchar('\n');
    }
}
