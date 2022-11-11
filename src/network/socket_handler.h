#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include "network/server_env.h"

void register_connection(struct server_env *env, int host_socket_fd);

void process_data(struct server_env *env, int event_index, char *data,
                  size_t size);

bool incoming_connection(struct server_env *env, int client_socket_fd);

void close_connection(struct server_env *env, int client_socket_fd);

#endif /* !SOCKET_HANDLER_H */
