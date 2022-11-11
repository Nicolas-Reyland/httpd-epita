#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <stdbool.h>
#include <stdio.h>

#include "utils/hash_map/hash_map.h"
#include "utils/string_utils.h"

struct server_config
{
    const char *filename;
    struct hash_map *global;
    size_t num_vhosts;
    struct hash_map **vhosts;
};

struct server_config *parse_server_config(const char *filename);

struct server_config *parse_server_config_from_stream(FILE *stream);

struct server_config *fill_server_config(struct server_config *config);

void free_server_config(struct server_config *config, bool free_obj);

#endif /* !CONFIG_PARSER_H */
