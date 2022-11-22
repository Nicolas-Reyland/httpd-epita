#include "client.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/state.h"

/*
 * On error, returns NULL
 */
struct client *init_client(struct vhost *vhost, int socket_fd, char *ip_addr)
{
    struct client *client = malloc(sizeof(struct client));
    if (client == NULL)
    {
        log_error("%s: Out of memory (client)\n", __func__);
        return NULL;
    }

    client->socket_fd = socket_fd;
    client->ip_addr = ip_addr;
    client->vhost = vhost;
    client->index = -1;
    client->buffered_data = NULL;
    client->buffered_size = 0;

    return client;
}

/*
 * Will NOT free the vhost
 */
void destroy_client(struct client *client, bool free_obj)
{
    // Close socket first
    epoll_ctl(g_state.env->epoll_fd, EPOLL_CTL_DEL, client->socket_fd, NULL);
    if (close(client->socket_fd) == -1)
        log_warn("%s(close): %s\n", __func__, strerror(errno));

    FREE_SET_NULL(client->ip_addr);
    FREE_SET_NULL(client->buffered_data);

    if (free_obj)
        FREE_SET_NULL(client);
}
