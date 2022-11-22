#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "network/vhost.h"

struct client
{
    int socket_fd;
    char *ip_addr;
    struct vhost *vhost;
    ssize_t index;
    char *buffered_data;
    size_t buffered_size;
};

struct client *init_client(struct vhost *vhost, int socket_fd, char *ip_addr);

void destroy_client(struct client *client, bool free_obj);

#endif /* !CLIENT_H */
