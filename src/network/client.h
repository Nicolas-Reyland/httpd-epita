#ifndef CLIENT_H
#define CLIENT_H

#include <pthread.h>
#include <stdbool.h>

#include "network/vhost.h"

struct client
{
    int socket_fd;
    char *ip_addr;
    struct vhost *vhost;
    pthread_mutex_t mutex;
};

struct client *init_client(struct vhost *vhost, int socket_fd, char *ip_addr);

struct client *retrieve_client(struct vhost *vhost, ssize_t index);

int release_client(struct client *client);

void destroy_client(struct client *client, bool free_obj);

#endif /* !CLIENT_H */
