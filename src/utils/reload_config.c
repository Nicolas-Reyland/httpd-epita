#include "reload_config.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "network/server.h"
#include "network/vhost.h"
#include "process/sig_handlers.h"
#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/parsers/config/config_parser.h"
#include "utils/state.h"

static int update_vhosts(struct server_config *new_config);

static int update_global(struct hash_map *new_global);

/*
 * Since the vhosts are ip-based, vhosts will be matched by their IPs, not
 * by their server-names. Vhosts may be ip-based, but they will be
 * recognized using their port, too.
 */
void reload_config(void)
{
    const char *filename = g_state.env->config->filename;
    // Parse and validate new config
    struct server_config *new_config = parse_server_config(filename);
    new_config = fill_server_config(new_config);
    if (new_config == NULL)
    {
        log_error("New config is invalid. Exiting.\n");
        free_server_config(new_config, true);
        graceful_shutdown();
    }

    // Update global parameters
    if (update_global(new_config->global) == -1)
    {
        free_server_config(new_config, true);
        graceful_shutdown();
    }

    // Update the existing vhosts
    if (update_vhosts(new_config) == -1)
    {
        free_server_config(new_config, true);
        graceful_shutdown();
    }
}

static int update_single_vhost(struct hash_map *new_vhost, size_t index,
                               signed char *markers);

static void reload_remove_vhost(size_t vhost_index);

static int reload_add_vhost(struct hash_map *new_vhost_map);

int update_vhosts(struct server_config *new_config)
{
    struct server_config *cur_config = g_state.env->config;
    signed char *markers = calloc(
        cur_config->num_vhosts + new_config->num_vhosts, sizeof(signed char));
    if (markers == NULL)
    {
        log_error("%s: Out of memory (markers)\n", __func__);
        return -1;
    }
    /* in markers:
     *  - 0 means that the vhost is NOT present in the config
     *  - 1 means that the vhost is present in the config
     *
     *  from 0 to cur_config->num_vhosts :
     *    - mark vhosts that are still present
     *  from cur_config->num_vhosts to (\1) + new_config->num_vhosts :
     *    - mark new vhosts, that were not present in the config
     */
    size_t num_to_add = 0;
    for (size_t index = 0; index < new_config->num_vhosts; ++index)
    {
        if (update_single_vhost(new_config->vhosts[index], index, markers)
            == -1)
            return -1;
        if (!markers[cur_config->num_vhosts + index])
            ++num_to_add;
    }

    // Remove the vhosts that are not present in the config file anymore
    size_t old_num_vhosts = cur_config->num_vhosts;
    size_t num_removed = 0;
    for (size_t i = 0; i < old_num_vhosts; ++i)
        if (!markers[i])
            reload_remove_vhost(i - num_removed++);

    // Allocate new space for vhosts
    size_t real_num_vhosts = old_num_vhosts + num_to_add - num_removed;
    void *new_vhosts =
        realloc(g_state.env->vhosts, real_num_vhosts * sizeof(struct vhost));
    void *new_vhost_maps = realloc(g_state.env->config->vhosts,
                                   real_num_vhosts * sizeof(struct hash_map *));
    if (new_vhosts == NULL || new_vhost_maps == NULL)
    {
        log_error("%s: Out of memory\n");
        free(new_vhosts);
        free(new_vhost_maps);
        return -1;
    }
    g_state.env->vhosts = new_vhosts;
    g_state.env->config->vhosts = new_vhost_maps;

    // Add the new vhosts, that were not present in the old config
    for (size_t i = 0; i < new_config->num_vhosts; ++i)
        if (!markers[old_num_vhosts + i]
            && reload_add_vhost(new_config->vhosts[i]) == -1)
            return -1;

    FREE_SET_NULL(new_config);
    FREE_SET_NULL(new_config->vhosts);
    FREE_SET_NULL(markers);
    return 0;
}

static int update_logging(struct hash_map *new_global);

int update_global(struct hash_map *new_global)
{
    // Update logging
    if (update_logging(new_global) == -1)
        return -1;

    free_hash_map(new_global, true);

    return 0;
}

int update_logging(struct hash_map *new_global)
{
    char *new_log_file = hash_map_get(new_global, "log_file");
    char *new_log = hash_map_get(new_global, "log");
    int new_logging = new_log == NULL || strcmp(new_log, "true") == 0;
    char *old_log_file = hash_map_get(g_state.env->config->global, "log_file");

    log_debug("Old debug: %d, New debug: %d\n", g_state.logging, new_logging);

    if (!g_state.logging && !new_logging)
        return 0;

    if (g_state.logging && new_logging)
    {
        // Both are stdout (NULL)
        if (old_log_file == NULL && new_log_file == NULL)
            return 0;
        // Same log file
        if (old_log_file != NULL && new_log_file != NULL
            && strcmp(old_log_file, new_log_file) == 0)
            return 0;

        // Close the old stream, if it isn't stdout
        if (g_state.log_file_stream != stdout
            && fclose(g_state.log_file_stream) == EOF)
        {
            log_error("%s: %s\n", __func__, strerror(errno));
            return -1;
        }
        g_state.log_file_stream = NULL;
        g_state.logging = false;
    }
    /* FALLTHROUGH */
    if (!g_state.logging && new_logging)
    {
        log_debug("Opening \"%s\" as new log file\n", new_log_file);
        FILE *new_log_file_stream =
            new_log_file == NULL ? stdout : fopen(new_log_file, "w");
        if (new_log_file_stream == NULL)
        {
            log_error("%s: could not open new log file \"%s\"\n", __func__,
                      new_log_file);
            return -1;
        }
        g_state.log_file_stream = new_log_file_stream;
        g_state.logging = new_logging;

        return 0;
    }
    // Only remaining possiblity : g_state.logging && !new_logging
    if (g_state.log_file_stream != stdout
        && fclose(g_state.log_file_stream) == EOF)
    {
        log_error("%s: %s\n", __func__, strerror(errno));
        return -1;
    }
    g_state.log_file_stream = NULL;
    g_state.logging = false;

    return 0;
}

int update_single_vhost(struct hash_map *new_vhost, size_t index,
                        signed char *markers)
{
    log_debug("Reloading ...\n");
    size_t num_cur_vhosts = g_state.env->config->num_vhosts;
    struct hash_map **cur_vhosts = g_state.env->config->vhosts;

    /*
     * First, check if the ip/port pair is already registered to an
     * existing vhost
     */
    char *new_ip = hash_map_get(new_vhost, "ip");
    char *new_port = hash_map_get(new_vhost, "port");
    if (new_ip == NULL || new_port == NULL)
    {
        log_error("%s: Either ip or port is missing in new config\n", __func__);
        return -1;
    }

    for (size_t i = 0; i < num_cur_vhosts; ++i)
    {
        if (markers[i])
            continue;

        struct hash_map *cur_vhost = cur_vhosts[i];
        char *cur_ip = hash_map_get(cur_vhost, "ip");
        char *cur_port = hash_map_get(cur_vhost, "port");
        if (cur_ip == NULL || cur_port == NULL)
            continue;

        if (strcmp(new_ip, cur_ip) == 0 && strcmp(new_port, cur_port) == 0)
        {
            log_debug("Found match for %s:%s\n", new_ip, new_port);
            // Match found !
            markers[i] = 1;
            markers[num_cur_vhosts + index] = 1;
            // Replace old vhsot with the new one
            free_hash_map(cur_vhost, true);
            cur_vhosts[i] = new_vhost;
            (g_state.env->vhosts + i)->map = new_vhost;
            break;
        }
    }

    return 0;
}

void reload_remove_vhost(size_t vhost_index)
{
    log_debug("Removing vhost at index %zu\n", vhost_index);
    struct server_config *config = g_state.env->config;
    struct vhost *vhost = g_state.env->vhosts + vhost_index;

    memmove(config->vhosts + vhost_index, config->vhosts + vhost_index + 1,
            (--config->num_vhosts - vhost_index) * sizeof(struct hash_map *));
    free_vhost(vhost, true, false);
    memmove(vhost, vhost + 1,
            (config->num_vhosts - vhost_index) * sizeof(struct vhost));
}

int reload_add_vhost(struct hash_map *new_vhost_map)
{
    char *vhost_ip = hash_map_get(new_vhost_map, "ip");
    char *vhost_port = hash_map_get(new_vhost_map, "port");
    log_debug("Adding new vhost @ %s:%s\n", vhost_ip, vhost_port);

    struct vhost new_vhost;
    if (init_vhost(new_vhost_map, &new_vhost) == -1)
    {
        log_error("%s: failed to init vhost @ %s:%s\n", __func__, vhost_ip,
                  vhost_port);
        return -1;
    }

    size_t index = g_state.env->config->num_vhosts++;
    g_state.env->config->vhosts[index] = new_vhost_map;
    g_state.env->vhosts[index] = new_vhost;

    return 0;
}
