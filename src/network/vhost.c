#include "vhost.h"

#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/state.h"
#include "utils/vector_client/vector_client.h"

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
        vhosts[i].clients = vector_client_init(VHOST_VECTOR_INIT_SIZE);
        // TODO: OOM check
        vhosts[i].map = config->vhosts[i];
    }

    return vhosts;
}

void free_vhost(struct vhost *vhost, bool free_map, bool free_obj)
{
    // and un-register socket fd
    epoll_ctl(g_state.env->epoll_fd, EPOLL_CTL_DEL, vhost->socket_fd, NULL);
    CLOSE_ALL(vhost->socket_fd);

    // destroy all clients (clllose fds and un-register from epoll)
    destroy_vector_client(vhost->clients);

    // additional frees
    if (free_map)
        free_hash_map(vhost->map, true);

    if (free_obj)
        free(vhost);
}
