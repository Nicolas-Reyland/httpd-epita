#include "vhost.h"

#include "utils/logging.h"
#include "utils/mem.h"

#define VHOST_VECTOR_INIT_SIZE 10

struct vhost *init_vhosts(struct server_config *config)
{
    struct vhost *vhosts = calloc(config->num_vhosts, sizeof(struct vhost));
    if (vhosts == NULL)
    {
        log_error("%s: Out of memory (vhosts)", __func__);
        return NULL;
    }
    for (size_t i = 0; i < config->num_vhosts; ++i)
    {
        vhosts[i].socket_fd = -1;
        vhosts[i].clients = vector_init(VHOST_VECTOR_INIT_SIZE);
        // TODO: OOM check
        vhosts[i].map = config->vhosts[i];
    }

    return vhosts;
}

void free_vhost(struct vhost *vhost, bool free_config, bool free_obj)
{
    CLOSE_ALL(vhost->socket_fd);
    if (free_config)
        free_hash_map(vhost->map, true);
    free_vector(vhost->clients);
    if (free_obj)
        free(vhost);
}