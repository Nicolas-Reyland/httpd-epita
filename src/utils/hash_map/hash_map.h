#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>
#include <stddef.h>

#include "utils/hash_map/pair_list.h"

struct hash_map
{
    // linear collection of the keys
    char **keys;
    size_t num_keys;
    // actual hashmap
    struct pair_list **data;
    size_t size;
};

size_t hash(char *str);

struct hash_map *hash_map_init(size_t size);

void free_hash_map(struct hash_map *hash_map, bool free_obj);

bool hash_map_insert(struct hash_map *hash_map, char *key, char *value,
                     bool *updated);

char *hash_map_get(const struct hash_map *hash_map, char *key);

bool hash_map_remove(struct hash_map *hash_map, char *key);

void hash_map_dump(struct hash_map *hash_map);

#endif /* !HASH_MAP_H */
