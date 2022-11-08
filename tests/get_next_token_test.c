#include <criterion/criterion.h>

#include "tests/test_utils/test_utils.h"
#include "utils/parsers/config/config_parser.h"

#define NUM_VHOSTS 1
Test(ParseConfig, simple)
{
    const char *filename = "content/config/simple.txt";
    // expected
    char *global_keys[] = { "a", "b", "c" };
    size_t global_num_keys = sizeof(global_keys) / sizeof(global_keys[0]);
    char *global_values[] = { "va", "vb", "vc" };

    char *vhost_keys[NUM_VHOSTS][] = {
        { "x", "y", "z" },
    };
    char *vhost_values[NUM_VHOSTS][] = {
        { "vx", "vy", "vz" },
    };
    size_t num_vhosts = sizeof(vhost_keys) / sizeof(void *);
    // actual
    struct server_config *actual = parse_config(filename);

    CR_ASSERT_NOT_NULL_EXPANDED(actual);
    CR_ASSERT_NOT_NULL_EXPANDED(actual->global);
    CR_ASSERT_EQ_DIGIT_EXPANDED(global_num_keys, actual->global->num_keys);
    for (size_t i = 0; i < global_num_keys; ++i)
    {
        CR_ASSERT_NOT_NULL_EXPANDED(actual->global->keys[i]);
        CR_ASSERT_STR_EQ_EXPANDED(actual->global->keys[i], global_values[i]);
    }
    CR_ASSERT_NOT_NULL_EXPANDED(actual->vhosts);
    CR_ASSERT_EQ_DIGIT_EXPANDED(vhost_keys, actual->num_vhosts);
    for (size_t i = 0; i < global_num_keys; ++i)
    {
        struct hash_map *map = actual->vhosts[i];
        CR_ASSERT_NOT_NULL_EXPANDED(map);
        CR_ASSERT_EQ_DIGIT_EXPANDED(, map->num_keys);
        for (size_t i = 0; i < global_num_keys; ++i)
        {
            CR_ASSERT_NOT_NULL_EXPANDED(actual->global->keys[i]);
            CR_ASSERT_STR_EQ_EXPANDED(actual->global->keys[i],
                                      global_values[i]);
        }
    }
}
#undef NUM_VHOSTS
