#include "server_env.h"

#include <stdlib.h>
#include <unistd.h>

#include "utils/mem.h"

void free_server_env(struct server_env *env, bool close_fds, bool free_obj)
{
    if (close_fds)
    {
        CLOSE_ALL(env->epoll_fd);
        for (size_t i = 0; i < env->config->num_vhosts; ++i)
        {
            CLOSE_ALL(env->vhosts_socket_fds[i]);
        }
    }
    free_server_config(env->config, true);
    free(env->events);
    if (free_obj)
        free(env);
}
