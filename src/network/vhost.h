#ifndef VHOST_H
#define VHOST_H

#include <stdbool.h>
#include <stdlib.h>

#include "utils/hash_map/hash_map.h"
#include "utils/parsers/config/config_parser.h"
#include "utils/vector/vector.h"

struct vhost
{
    int socket_fd;
    struct vector *clients;
    struct hash_map *map;
};

struct vhost *init_vhosts(struct server_config *config);

void free_vhost(struct vhost *vhost, bool free_config, bool free_obj);

#endif /* VHOST_H */
