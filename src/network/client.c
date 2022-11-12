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

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    // Scope the 'error' variable
    {
        int error;
        if ((error = pthread_mutex_init(&mutex, NULL)))
        {
            FREE_SET_NULL(client);
            log_error("%s(pthread_mutex_init): %s\n", strerror(error));
            return NULL;
        }
    }

    client->socket_fd = socket_fd;
    client->ip_addr = ip_addr;
    client->vhost = vhost;
    client->mutex = mutex;

    return client;
}

struct client *retrieve_client(struct vhost *vhost, ssize_t index)
{
    if (!vector_client_valid_index(vhost->clients, index))
        return NULL;

    {
        int error;
        if ((error = pthread_mutex_lock(&vhost->clients->data[index]->mutex)))
        {
            log_error("Unable to lock mutex (%s) for client at index %ld in "
                      "vhost %s\n",
                      strerror(error), hash_map_get(vhost->map, "server_name"));
            return NULL;
        }
    }

    // Redo the read (there may have been a race condition on values other than
    // the mutex itself)
    return vhost->clients->data[index];
}

int release_client(struct client *client)
{
    int error;
    if ((error = pthread_mutex_unlock(&client->mutex)) != 0)
    {
        log_error("Unable to unlock mutex (%s) for client with socket fd %d\n",
                  strerror(error), client->socket_fd);
        return -1;
    }

    return 0;
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

    {
        int error;
        if ((error = pthread_mutex_destroy(&client->mutex)))
            log_warn("%s(pthread_mutex_destroy): %s\n", __func__,
                     strerror(error));
    }

    if (free_obj)
        FREE_SET_NULL(client);
}
