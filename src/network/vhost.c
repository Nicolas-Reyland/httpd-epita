#include "network/vhost.h"

void free_vhost(struct vhost *vhost, bool free_config, bool free_obj)
{
    CLOSE_ALL(vhost->socket_fd);
    if (free_config)
        free_hash_map(vhost->config, true);
    free_vector(vhost->clients);
    if (free_obj)
        free(vhost);
}
