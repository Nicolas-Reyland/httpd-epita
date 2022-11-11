#include "vhost.h"

#include <string.h>
#include <unistd.h>

#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/state.h"

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
        vhosts[i].client_ips =
            g_state.logging ? vector_str_init(VHOST_VECTOR_INIT_SIZE) : NULL;
    }

    return vhosts;
}

void free_vhost(struct vhost *vhost, bool free_config, bool free_obj)
{
    CLOSE_ALL(vhost->socket_fd);
    if (free_config)
        free_hash_map(vhost->map, true);
    for (size_t i = 0; i < vhost->clients->size; ++i)
        close(vhost->clients->data[i]);
    free_vector(vhost->clients);
    free_vector_str(vhost->client_ips);
    if (free_obj)
        free(vhost);
}
