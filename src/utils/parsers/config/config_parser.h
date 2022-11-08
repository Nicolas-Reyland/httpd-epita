#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <stdio.h>

#include "utils/hash_map/hash_map.h"
#include "utils/string_utils.h"

struct server_config
{
    struct hash_map *global;
    size_t num_vhosts;
    struct hash_map **vhosts;
};

struct server_config *parse_config(char *filename);

struct server_config *parse_config_from_stream(FILE *stream);

void free_server_config(struct server_config *config, bool free_obj);

#endif /* CONFIG_PARSER_H */
