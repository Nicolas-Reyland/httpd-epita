#ifndef SOCKET_HANDLER_H
#define SOCKET_HANDLER_H

#include <stdbool.h>
#include <sys/types.h>

#include "network/client.h"

void register_connection(int host_socket_fd);

void process_data(struct client *client, char *data, size_t size);

ssize_t incoming_connection(int client_socket_fd);

void close_connection(int client_socket_fd);

struct vhost *vhost_from_host_socket(int socket_fd);

struct client *client_from_client_socket(int socket_fd, bool wait);

#endif /* !SOCKET_HANDLER_H */
