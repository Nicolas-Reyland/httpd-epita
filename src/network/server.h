#ifndef SERVER_H
#define SERVER_H

#include <stdnoreturn.h>

#include "network/server_env.h"
#include "utils/parsers/config/config_parser.h"

_Noreturn void start_all(struct server_config *config, char *pid_file);

_Noreturn void run_server(struct server_env *env);

int setup_socket(int epoll_fd, char *ip_addr, char *port);

#endif /* !SERVER_H */
