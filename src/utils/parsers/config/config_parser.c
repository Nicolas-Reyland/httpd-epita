#include "config_parser.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "utils/mem.h"
#include "utils/string_utils.h"

#define HASH_MAP_SIZE 3

struct server_config *parse_server_config(const char *filename)
{
    FILE *stream = fopen(filename, "r");
    if (stream == NULL)
        return NULL;

    return parse_server_config_from_stream(stream);
}

static bool parse_config_raw(struct server_config *config, char **lines);

struct server_config *parse_server_config_from_stream(FILE *stream)
{
    // read the content
    size_t num_lines;
    char **lines = read_lines_from_stream(stream, &num_lines);
    if (lines == NULL)
        return NULL;

    // allocate a config
    struct server_config *config = malloc(sizeof(struct server_config));
    if (config == NULL)
        return NULL;
    config->global = NULL;
    config->num_vhosts = 0;
    config->vhosts = NULL;

    // parse the content 'into' the config
    bool success = parse_config_raw(config, lines);

    // clean up
    for (size_t i = 0; i < num_lines; ++i)
        free(lines[i]);
    free(lines);
    FCLOSE_SET_NULL(stream);

    if (!success)
    {
        free_server_config(config, true);
        return NULL;
    }

    return config;
}

static struct hash_map *parse_attributes(char ***lines);

bool parse_config_raw(struct server_config *config, char **lines)
{
    while (line_is_empty(*lines))
        ++lines;

    bool global_is_set = false;
    while (lines[0] != NULL)
    {
        bool is_vhost;
        ;
        if (strcmp(lines[0], "[global]") == 0)
        {
            // global
            if (global_is_set)
                return false;
            global_is_set = true;
            is_vhost = false;
        }
        else if (strcmp(lines[0], "[[vhosts]]") == 0)
        {
            // vhosts
            is_vhost = true;
        }
        else
            return false;

        ++lines;
        struct hash_map *map = parse_attributes(&lines);
        if (is_vhost)
        {
            config->vhosts =
                realloc(config->vhosts,
                        (config->num_vhosts + 1) * sizeof(struct hash_map));
            config->vhosts[config->num_vhosts] = map;
            ++config->num_vhosts;
        }
        else
            config->global = map;
    }

    return true;
}

static int isnotseparator(int c)
{
    return !isspace(c) && c != '=';
}

struct hash_map *parse_attributes(char ***lines)
{
    if (lines == NULL && *lines == NULL)
        return NULL;

    struct hash_map *map = hash_map_init(HASH_MAP_SIZE);
    if (map == NULL)
        return NULL;
    while ((*lines)[0] != NULL && (*lines)[0][0] != '[')
    {
        if (line_is_empty(**lines))
        {
            ++(*lines);
            continue;
        }

        char *line_cpy = **lines;
        skip_to_nonwhitespace(*lines);
        char *key = token_from_class(*lines, &isnotseparator, NULL);
        if (lines[0][0] == 0)
        {
            free_hash_map(map, true);
            **lines = line_cpy;
            return NULL;
        }
        ++(**lines);
        char *value = token_from_class(*lines, NULL, NULL);

        hash_map_insert(map, key, value, NULL);
    }

    return map;
}

/*
 * Returns true if the config is valid, false otherwise.
 * Fills the config 'global' and 'vhost's with default values
 */
bool fill_server_config(struct server_config *config,
                        struct hash_map *default_global,
                        struct hash_map *default_vhost)
{
    (void)config;
    (void)default_global;
    (void)default_vhost;
    return true;
}

void free_server_config(struct server_config *config, bool free_obj)
{
    if (config == NULL)
        return;

    FREE_SET_NULL(config->global)

    for (size_t i = 0; i < config->num_vhosts; ++i)
    {
        free_hash_map(config->vhosts[i], true);
        config->vhosts[i] = NULL;
    }
    config->num_vhosts = 0;

    if (free_obj)
        free(config);
}
