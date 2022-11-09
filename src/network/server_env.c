#include "server_env.h"

#include <unistd.h>

void free_server_env(struct server_env *env, bool close_fds, bool free_obj)
{
    free_server_config(env->config, true);
    if (close_fds)
    {
        close(env->socket_fd);
        close(env->epoll_fd);
    }
    free(env->events);
    if (free_obj)
        free(env);
}
