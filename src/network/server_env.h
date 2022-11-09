#ifndef SERVER_ENV_H
#define SERVER_ENV_H

#include <stdbool.h>

#include "utils/parsers/config/config_parser.h"

struct server_env
{
    struct server_config *config;
    int socket_fd;
    int epoll_fd;
    struct epoll_event event;
    struct epoll_event *events;
    int num_threads;
};

void free_server_env(struct server_env *env, bool close_fds, bool free_obj);

#endif /* SERVER_ENV_H */
