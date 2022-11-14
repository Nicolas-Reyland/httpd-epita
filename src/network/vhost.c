#include "vhost.h"

#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "multithreading/mutex_wrappers.h"
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
        log_error("%s: Out of memory (init vhosts)", __func__);
        return NULL;
    }
    for (size_t i = 0; i < config->num_vhosts; ++i)
    {
        if (init_vhost(config->vhosts[i], vhosts + i) == -1)
        {
            log_error("%s(init vhost): failed to initialize vhosts\n");
            for (size_t j = 0; j < i; ++j)
                free_vhost(vhosts + i, false, false);
            return NULL;
        }
    }

    return vhosts;
}

int init_vhost(struct hash_map *map, struct vhost *vhost)
{
    // TODO: OOM checks

    struct vhost tmp = {
        .clients_mutex = PTHREAD_MUTEX_INITIALIZER,
        .socket_fd = -1,
        .clients = NULL,
        .map = map,
    };
    *vhost = tmp;

    {
        int error;
        if ((error = init_mutex_wrapper(&vhost->clients_mutex)))
        {
            log_error("%s(clients mutex init); %s\n", __func__,
                      strerror(error));
            return -1;
        }
    }

    vhost->clients = vector_client_init(VHOST_VECTOR_INIT_SIZE);
    if (vhost->clients == NULL)
    {
        return -1;
    }

    return 0;
}

void free_vhost(struct vhost *vhost, bool free_map, bool free_obj)
{
    if (vhost == NULL)
        return;

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
