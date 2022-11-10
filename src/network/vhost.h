#ifndef VHOST_H
#define VHOST_H

#include <stdbool.h>

#include "utils/hash_map/hash_map.h"
#include "utils/vector/vector.h"

struct vhost
{
    int socket_fd;
    struct vector *clients;
    struct hash_map *config;
};

void free_vhost(struct vhost *vhost, bool free_obj);

#endif /* VHOST_H */
