#ifndef VHOST_H
#define VHOST_H

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utils/hash_map/hash_map.h"
#include "utils/parsers/config/config_parser.h"
#include "utils/vectors/vector/vector.h"
#include "utils/vectors/vector_str/vector_str.h"

#define VHOST_VECTOR_INIT_SIZE 10

struct vhost
{
    int socket_fd;
    struct vector *clients;
    struct hash_map *map;
    struct vector_str *client_ips;
    pthread_mutex_t *mutexes;
};

struct vhost *init_vhosts(struct server_config *config);

void free_vhost(struct vhost *vhost, bool free_config, bool free_obj);

#endif /* VHOST_H */
