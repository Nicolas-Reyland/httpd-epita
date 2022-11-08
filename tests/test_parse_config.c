#include <criterion/criterion.h>
#include <stdbool.h>

#include "tests/test_utils/test_utils.h"
#include "utils/parsers/config/config_parser.h"

#define MAX_NUM_KEYS 10

static size_t length_of_null_terminated(void **arr);

#define NUM_VHOSTS 1
Test(ParseConfig, simple)
{
    const char *filename = "content/config/simple.txt";
    // expected
    char *global_keys[] = { "a", "b", "c" };
    size_t global_num_keys = sizeof(global_keys) / sizeof(global_keys[0]);
    char *global_values[] = { "va", "vb", "vc" };

    char *vhost_keys[NUM_VHOSTS][MAX_NUM_KEYS] = {
        { "x", "y", "z", NULL },
    };
    char *vhost_values[NUM_VHOSTS][MAX_NUM_KEYS] = {
        { "vx", "vy", "vz", NULL },
    };
    // actual
    struct server_config *actual = parse_config(filename);

    CR_ASSERT_NOT_NULL_EXPANDED(actual);
    CR_ASSERT_NOT_NULL_EXPANDED(actual->global);
    CR_ASSERT_EQ_DIGIT_EXPANDED(global_num_keys, actual->global->num_keys);
    for (size_t i = 0; i < global_num_keys; ++i)
    {
        char *key = actual->global->keys[i];
        CR_ASSERT_NOT_NULL_EXPANDED(key);
        CR_ASSERT_STR_EQ_EXPANDED(global_keys[i], key);
        char *actual_global_value = hash_map_get(actual->global, key);
        CR_ASSERT_STR_EQ_EXPANDED(global_values[i], actual_global_value);
    }
    free_hash_map(actual->global, true);
    CR_ASSERT_NOT_NULL_EXPANDED(actual->vhosts);
    CR_ASSERT_EQ_DIGIT_EXPANDED(NUM_VHOSTS, actual->num_vhosts);
    for (size_t i = 0; i < NUM_VHOSTS; ++i)
    {
        struct hash_map *map = actual->vhosts[i];
        CR_ASSERT_NOT_NULL_EXPANDED(map);
        void *void_arr = vhost_keys[i];
        size_t expected_num_keys = length_of_null_terminated(void_arr);
        CR_ASSERT_EQ_DIGIT_EXPANDED(expected_num_keys, map->num_keys);
        for (size_t j = 0; j < expected_num_keys; ++j)
        {
            char *key = actual->vhosts[i]->keys[j];
            CR_ASSERT_NOT_NULL_EXPANDED(key);
            CR_ASSERT_STR_EQ_EXPANDED(vhost_keys[i][j], key);
            char *actual_vhost_value = hash_map_get(actual->vhosts[i], key);
            CR_ASSERT_STR_EQ_EXPANDED(vhost_values[i][j], actual_vhost_value);
        }
        free_hash_map(map, true);
    }
    free(actual->vhosts);
    free(actual);
}
#undef NUM_VHOSTS

size_t length_of_null_terminated(void **arr)
{
    if (arr == NULL)
        return 0;

    size_t size = 0;
    for (; arr[size] != NULL; ++size)
        continue;

    return size;
}
