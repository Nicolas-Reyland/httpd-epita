#include "config_parser.h"


#include "utils/mem.h"
#include "utils/string_utils.h"

struct server_config *parse_config(char *filename)
{
    FILE *stream = fopen(filename, "r");
    if (stream == NULL)
        return NULL;

    return parse_config_from_stream(stream);
}

struct server_config *parse_config_from_stream(FILE *stream)
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
    //

    // clean up
    free_array(lines, num_lines, true);
    FCLOSE_SET_NULL(stream);

    return config;
}

/*
 * Returns true if the config is valid, false otherwise.
 * Fills the config 'global' and 'vhost's with default values
 */
bool fill_server_config(struct server_config *config, struct hash_map *default_global, struct hash_map *default_vhost)
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
        hash_map_free(congig->vhosts[i], true);
        congig->vhosts[i] = NULL;
    }
    config->num_vhosts = 0;

    if (free_obj)
        free(config);
}
