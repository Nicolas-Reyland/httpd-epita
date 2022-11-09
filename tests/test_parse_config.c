#include <criterion/criterion.h>
#include <stdbool.h>

#include "tests/test_utils/test_utils.h"
#include "utils/parsers/config/config_parser.h"

#define MAX_NUM_KEYS 10

#define CR_ASSERT_HASH_MAP_EQ(NumKeys, Keys, Values, HashMap)                  \
    CR_ASSERT_NOT_NULL_EXPANDED(HashMap);                                      \
    CR_ASSERT_EQ_DIGIT_EXPANDED(NumKeys, (HashMap)->num_keys);                 \
    for (size_t i = 0; i < NumKeys; ++i)                                       \
    {                                                                          \
        char *key = (HashMap)->keys[i];                                        \
        CR_ASSERT_NOT_NULL_EXPANDED(key);                                      \
        CR_ASSERT_STR_EQ_EXPANDED(Keys[i], key);                               \
        char *actual_value = hash_map_get((HashMap), key);                     \
        CR_ASSERT_STR_EQ_EXPANDED(Values[i], actual_value);                    \
    }

#define TEST_SERVER_CONFIG                                                     \
    CR_ASSERT_NOT_NULL_EXPANDED(actual);                                       \
    CR_ASSERT_HASH_MAP_EQ(global_num_keys, global_keys, global_values,         \
                          actual->global)                                      \
    free_hash_map(actual->global, true);                                       \
    CR_ASSERT_NOT_NULL_EXPANDED(actual->vhosts);                               \
    CR_ASSERT_EQ_DIGIT_EXPANDED(NUM_VHOSTS, actual->num_vhosts);               \
    for (size_t i = 0; i < NUM_VHOSTS; ++i)                                    \
    {                                                                          \
        struct hash_map *map = actual->vhosts[i];                              \
        CR_ASSERT_NOT_NULL_EXPANDED(map);                                      \
        void *void_arr = vhost_keys[i];                                        \
        size_t expected_num_keys = length_of_null_terminated(void_arr);        \
        CR_ASSERT_EQ_DIGIT_EXPANDED(expected_num_keys, map->num_keys);         \
        for (size_t j = 0; j < expected_num_keys; ++j)                         \
        {                                                                      \
            char *key = actual->vhosts[i]->keys[j];                            \
            CR_ASSERT_NOT_NULL_EXPANDED(key);                                  \
            CR_ASSERT_STR_EQ_EXPANDED(vhost_keys[i][j], key);                  \
            char *actual_vhost_value = hash_map_get(actual->vhosts[i], key);   \
            CR_ASSERT_STR_EQ_EXPANDED(vhost_values[i][j], actual_vhost_value); \
        }                                                                      \
        free_hash_map(map, true);                                              \
    }                                                                          \
    free(actual->vhosts);                                                      \
    free(actual);

static size_t length_of_null_terminated(void **arr);

#define NUM_VHOSTS 1
Test(ParseConfig, simple)
{
    const char *filename = "tests/content/simple.conf";
    // expected
    char *global_keys[] = { "log_file", "log", "pid_file" };
    size_t global_num_keys = sizeof(global_keys) / sizeof(global_keys[0]);
    char *global_values[] = { "server.log", "true", "/tmp/HTTPd.pid" };

    char *vhost_keys[NUM_VHOSTS][MAX_NUM_KEYS] = {
        { "server_name", "port", "ip", "root_dir", NULL },
    };
    char *vhost_values[NUM_VHOSTS][MAX_NUM_KEYS] = {
        { "images", "1312", "127.0.0.1", "votai/test.", NULL },
    };
    // actual
    struct server_config *actual = parse_server_config(filename);

    TEST_SERVER_CONFIG
}
#undef NUM_VHOSTS

#define NUM_VHOSTS 2
Test(ParseConfig, two_vhosts)
{
    const char *filename = "tests/content/two_vhosts.conf";
    // expected
    char *global_keys[] = { "log_file", "log", "pid_file" };
    size_t global_num_keys = sizeof(global_keys) / sizeof(global_keys[0]);
    char *global_values[] = { "server.log", "true", "/tmp/HTTPd.pid" };

    char *vhost_keys[NUM_VHOSTS][MAX_NUM_KEYS] = {
        { "server_name", "port", "ip", "root_dir", NULL },
        { "port", "root_dir", "ip", "server_name", NULL },
    };
    char *vhost_values[NUM_VHOSTS][MAX_NUM_KEYS] = {
        { "images", "1312", "127.0.0.1", "votai/test.", NULL },
        { "69", "/", "8.8.8.8", "videos" },
    };
    // actual
    struct server_config *actual = parse_server_config(filename);

    TEST_SERVER_CONFIG
}
#undef NUM_VHOSTS

#define NUM_VHOSTS 0
Test(ParseConfig, no_vhosts)
{
    const char *filename = "tests/content/no_vhosts.conf";
    // expected
    char *global_keys[] = { "log_file", "log", "pid_file" };
    size_t global_num_keys = sizeof(global_keys) / sizeof(global_keys[0]);
    char *global_values[] = { "server.log", "false", "/tmp/HTTPd.pid" };

    // actual
    struct server_config *actual = parse_server_config(filename);

    CR_ASSERT_HASH_MAP_EQ(global_num_keys, global_keys, global_values,
                          actual->global)

    CR_ASSERT_EQ_DIGIT_EXPANDED(0, actual->num_vhosts);
    CR_ASSERT_NULL_EXPANDED(actual->vhosts);

    free_server_config(actual, true);
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
