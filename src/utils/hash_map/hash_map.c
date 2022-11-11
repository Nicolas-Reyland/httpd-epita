#include "hash_map.h"

#include <stdlib.h>
#include <string.h>

struct hash_map *hash_map_init(size_t size)
{
    struct hash_map *map = malloc(sizeof(struct hash_map));
    if (map == NULL)
        return NULL;

    map->keys = NULL;
    map->num_keys = 0;
    map->size = size;
    map->data = calloc(size, sizeof(struct pair_list *));
    if (map->data == NULL)
    {
        free(map);
        return NULL;
    }

    return map;
}

void free_hash_map(struct hash_map *hash_map, bool free_obj)
{
    if (hash_map == NULL)
        return;

    for (size_t i = 0; i < hash_map->size; ++i)
        free_pair_list(hash_map->data[i]);

    free(hash_map->keys);
    free(hash_map->data);
    if (free_obj)
        free(hash_map);
}

bool hash_map_insert(struct hash_map *hash_map, char *key, char *value,
                     bool *updated)
{
    if (hash_map == NULL || hash_map->size == 0)
        return false;

    if (updated != NULL)
        *updated = false;

    size_t hash_of_key = hash(key);
    hash_of_key %= hash_map->size;

    // adding key to key
    hash_map->keys =
        realloc(hash_map->keys, (hash_map->num_keys + 1) * sizeof(char *));
    hash_map->keys[hash_map->num_keys++] = key;

    // printf("hash value: %zu\n", hash_of_key);
    struct pair_list *list = hash_map->data[hash_of_key];
    if (list == NULL)
    {
        // printf("adding new list\n");
        // add element to the hash-map
        hash_map->data[hash_of_key] = malloc(sizeof(struct pair_list));
        if (hash_map->data[hash_of_key] == NULL)
            return false;
        if (updated != NULL)
            *updated = false;
        hash_map->data[hash_of_key]->key = key;
        hash_map->data[hash_of_key]->value = value;
        hash_map->data[hash_of_key]->next = NULL;
    }
    else
    {
        // printf("updating list\n");
        // add element to the list (or update an element of the list)
        do
        {
            if (strcmp(key, list->key) == 0)
            {
                // printf("updating value\n");
                // update value
                if (updated != NULL)
                    *updated = true;
                // free old value
                free(list->value);
                list->value = value;
                return true;
            }
            list = list->next;
        } while (list != NULL);

        // printf("adding new value to list\n");

        // add element at the head of the list
        struct pair_list *new_list = malloc(sizeof(struct pair_list));
        if (new_list == NULL)
            return false;
        new_list->key = key;
        new_list->value = value;
        new_list->next = hash_map->data[hash_of_key];
        hash_map->data[hash_of_key] = new_list;
    }
    return true;
}

char *hash_map_get(const struct hash_map *hash_map, char *key)
{
    if (hash_map == NULL)
        return NULL;

    for (size_t i = 0; i < hash_map->size; ++i)
    {
        for (struct pair_list *list = hash_map->data[i]; list != NULL;
             list = list->next)
            if (strcmp(key, list->key) == 0)
                return list->value;
    }

    return NULL;
}

bool hash_map_remove(struct hash_map *hash_map, char *key)
{
    if (hash_map == NULL)
        return false;

    for (size_t i = 0; i < hash_map->size; ++i)
    {
        struct pair_list *list = hash_map->data[i];
        struct pair_list *prev = NULL;
        for (; list != NULL; list = list->next)
        {
            if (strcmp(key, list->key) == 0)
            {
                // remove this pair from the hashmap
                struct pair_list *next = list->next;
                list->next = NULL;
                free_pair_list(list);
                if (prev == NULL)
                    hash_map->data[i] = next;
                else
                    prev->next = next;
                return true;
            }
            prev = list;
        }
    }
    return false;
}
