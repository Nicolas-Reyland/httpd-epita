#include "thread_safe_write.h"

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#include "utils/logging.h"

static int retrieve_client_socket_fd(struct vhost *vhost, ssize_t index);

static int release_client_socket_fd(struct vhost *vhost, ssize_t index);

int thread_safe_write(struct vhost *vhost, ssize_t index, struct response *resp)
{
    int client_socket_fd = retrieve_client_socket_fd(vhost, index);
    if (client_socket_fd == -1)
    {
        log_error("%s: could not retrieve client socket fd %lu\n", __func__,
                  index);
        return -1;
    }

    size_t total_num_written = 0;
    ssize_t num_written = 0;
    while ((num_written = write(client_socket_fd, resp->res + total_num_written,
                                resp->res_len - total_num_written))
               != -1
           && total_num_written < resp->res_len)
        total_num_written += num_written;
    if (num_written == -1)
    {
        log_error("%s: %s", __func__, strerror(errno));
        return -1;
    }

    if (release_client_socket_fd(vhost, index) == -1)
    {
        log_error("%s: could not releast client socket fd %lu\n", __func__,
                  index);
        return -1;
    }

    return 0;
}

int retrieve_client_socket_fd(struct vhost *vhost, ssize_t index)
{
    size_t index_t = index;
    if (index == -1 || index_t >= vhost->clients->size)
        return -1;

    pthread_mutex_t mutex = vhost->mutexes[index];
    int error;
    if ((error = pthread_mutex_lock(&mutex)) != 0)
    {
        log_error(
            "Unable to lock mutex (%d) for client at index %ld in vhost %s\n",
            error, hash_map_get(vhost->map, "server_name"));
        return -1;
    }

    return 0;
}

int release_client_socket_fd(struct vhost *vhost, ssize_t index)
{
    size_t index_t = index;
    if (index == -1 || index_t >= vhost->clients->size)
        return -1;

    pthread_mutex_t mutex = vhost->mutexes[index];
    int error;
    if ((error == pthread_mutex_unlock(&mutex)) != 0)
    {
        log_error(
            "Unable to unlock mutex (%d) for client at index %ld in vhost %s\n",
            error, hash_map_get(vhost->map, "server_name"));
        return -1;
    }

    return 0;
}
