#include "config_parser.h"

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/string_utils.h"

#define HASH_MAP_SIZE 3

struct server_config *parse_server_config(const char *filename)
{
    FILE *stream = fopen(filename, "r");
    if (stream == NULL)
        return NULL;

    struct server_config *config = parse_server_config_from_stream(stream);
    fclose(stream);

    config->filename = filename;
    return config;
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
    if (config != NULL)
    {
        config->filename = NULL;
        config->global = NULL;
        config->num_vhosts = 0;
        config->vhosts = NULL;

        // parse the content 'into' the config
        if (!parse_config_raw(config, lines))
            free_server_config(config, true);
    }

    // clean up
    for (size_t i = 0; i < num_lines; ++i)
        free(lines[i]);
    free(lines);

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
        // global
        if (strcmp(lines[0], "[global]") == 0)
        {
            if (global_is_set)
                return false;
            global_is_set = true;
            is_vhost = false;
        }
        // vhosts
        else if (strcmp(lines[0], "[[vhosts]]") == 0)
            is_vhost = true;
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

static int isseparator(int c)
{
    return isspace(c) || c == '=';
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
        skip_all_classifier(*lines, &isseparator);
        if (lines[0][0] == 0)
        {
            free_hash_map(map, true);
            **lines = line_cpy;
            return NULL;
        }
        char *value = token_from_class(*lines, NULL, NULL);

        **lines = line_cpy;
        ++(*lines);

        hash_map_insert(map, key, value, NULL);
    }

    return map;
}

static void insert_if_not_present(struct hash_map *map, char *key, char *value);

void fill_root_dir(struct hash_map *vhost_map);

/*
 * Returns true if the config is valid, false otherwise.
 * Fills the config 'global' and 'vhost's with default values
 */
struct server_config *fill_server_config(struct server_config *config)
{
    if (config == NULL)
        return NULL;

    if (hash_map_get(config->global, "pid_file") == NULL)
    {
        log_server("%s: missing mandatory key in global: 'pid_file'\n",
                   config->filename);
        free_server_config(config, true);
        return NULL;
    };
    insert_if_not_present(config->global, "log", "true");

    char *mandatory_keys[] = { "server_name", "port", "ip", "root_dir" };
    for (size_t i = 0; i < config->num_vhosts; ++i)
    {
        struct hash_map *vhost_map = config->vhosts[i];
        for (size_t j = 0;
             j < sizeof(mandatory_keys) / sizeof(mandatory_keys[0]); ++j)
        {
            char *key = mandatory_keys[j];
            if (hash_map_get(vhost_map, key) == NULL)
            {
                log_server("%s: missing mandatory key in vhost[%zu]: '%s'\n",
                           config->filename, i, key);
                free_server_config(config, true);
                return NULL;
            };
        }
        insert_if_not_present(vhost_map, "default_file", "index.html");
        fill_root_dir(vhost_map);
    }

    return config;
}

void fill_root_dir(struct hash_map *vhost_map)
{
    char *root_dir = hash_map_get(vhost_map, "root_dir");
    char *path_buffer = malloc(PATH_MAX);
    if (root_dir[0] == '~')
    {
        char *home = getenv("HOME");
        char *new_root_dir = malloc(strlen(root_dir) + strlen(home) + 1);
        new_root_dir = strcpy(new_root_dir, home);
        root_dir++;
        strcat(new_root_dir, root_dir);
        char *resolved_path = realpath(new_root_dir, path_buffer);
        if (resolved_path == NULL)
        {
            free(path_buffer);
            hash_map_remove(vhost_map, "root_dir");
            return;
        }
        resolved_path = realloc(resolved_path, strlen(resolved_path) + 1);
        hash_map_insert(vhost_map, "root_dir", resolved_path, NULL);
        free(new_root_dir);
    }
    else
    {
        char *resolved_path = realpath(root_dir, path_buffer);
        if (resolved_path == NULL)
        {
            free(path_buffer);
            hash_map_remove(vhost_map, "root_dir");
            return;
        }
        resolved_path = realloc(resolved_path, strlen(resolved_path) + 1);
        hash_map_insert(vhost_map, "root_dir", resolved_path, NULL);
    }
}

void insert_if_not_present(struct hash_map *map, char *key, char *value)
{
    if (hash_map_get(map, key) == NULL)
        hash_map_insert(map, strdup(key), strdup(value), NULL);
}

void free_server_config(struct server_config *config, bool free_obj)
{
    if (config == NULL)
        return;

    free_hash_map(config->global, true);
    config->global = NULL;

    for (size_t i = 0; i < config->num_vhosts; ++i)
    {
        free_hash_map(config->vhosts[i], true);
        config->vhosts[i] = NULL;
    }
    config->num_vhosts = 0;
    FREE_SET_NULL(config->vhosts)

    if (free_obj)
        free(config);
}
