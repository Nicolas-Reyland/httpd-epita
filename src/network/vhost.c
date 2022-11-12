#include "vhost.h"

#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/state.h"
#include "utils/vectors/vector/vector.h"
#include "utils/vectors/vector_mutex/vector_mutex.h"
#include "utils/vectors/vector_str/vector_str.h"

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
        vhosts[i].mutexes = vector_mutex_init(VHOST_VECTOR_INIT_SIZE);
    }

    return vhosts;
}

void free_vhost(struct vhost *vhost, bool free_config, bool free_obj)
{
    // and un-register socket fd
    epoll_ctl(g_state.env->epoll_fd, EPOLL_CTL_DEL, vhost->socket_fd, NULL);
    CLOSE_ALL(vhost->socket_fd);
    for (size_t i = 0; i < vhost->clients->size; ++i)
    {
        int client_socket_fd = vhost->clients->data[i];
        epoll_ctl(g_state.env->epoll_fd, EPOLL_CTL_DEL, client_socket_fd, NULL);
        close(client_socket_fd);
    }

    // free buffers for fds and ips
    free_vector(vhost->clients);
    free_vector_str(vhost->client_ips);
    free_vector_mutex(vhost->mutexes);

    // additional frees
    if (free_config)
        free_hash_map(vhost->map, true);

    if (free_obj)
        free(vhost);
}
