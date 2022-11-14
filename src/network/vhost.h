#ifndef VHOST_H
#define VHOST_H

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utils/hash_map/hash_map.h"
#include "utils/parsers/config/config_parser.h"
#include "utils/vector_client/vector_client.h"

#define VHOST_VECTOR_INIT_SIZE 10

struct vhost
{
    int socket_fd;
    struct vector_client *clients;
    pthread_mutex_t clients_mutex;
    struct hash_map *map;
};

struct vhost *init_vhosts(struct server_config *config);

int init_vhost(struct hash_map *map, struct vhost *vhost);

void free_vhost(struct vhost *vhost, bool free_map, bool free_obj);

#endif /* !VHOST_H */
