#include "server_env.h"

#include <stdlib.h>

#include "network/vhost.h"
#include "utils/mem.h"

void free_server_env(struct server_env *env, bool close_fds, bool free_obj)
{
    if (close_fds)
    {
        close(env->epoll_fd);
    }
    for (size_t i = 0; i < env->config->num_vhosts; ++i)
        free_vhost(env->vhosts + i, false, false);
    FREE_SET_NULL(env->vhosts);

    free_server_config(env->config, true);
    free(env->events);
    if (free_obj)
        free(env);
}
