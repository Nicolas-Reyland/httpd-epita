#include <stdio.h>

#include "hash_map.h"

void hash_map_dump(struct hash_map *map, const char *prefix)
{
    if (map == NULL)
        return;

    for (size_t i = 0; i < map->num_keys; ++i)
        printf("%s%s: %s\n", prefix, map->keys[i],
               hash_map_get(map, map->keys[i]));

#if 0
    for (size_t i = 0; i < map->size; ++i)
    {
        struct pair_list *list = map->data[i];
        if (list == NULL)
            continue;
        printf("%s%s: %s", prefix == NULL ? "" : prefix, list->key,
               list->value);
        for (list = list->next; list != NULL; list = list->next)
            printf("\n%s%s: %s", prefix, list->key, list->value);
        putchar('\n');
    }
#endif /* 0 */
}
