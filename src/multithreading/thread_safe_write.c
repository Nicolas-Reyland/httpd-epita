#include "thread_safe_write.h"

#include <pthread.h>
#include <unistd.h>

#include "utils/logging.h"

int thread_safe_write(struct vhost *vhost, ssize_t index, struct response *resp)
{
    size_t index_t = index;
    if (index == -1 || index_t >= vhost->clients->size)
    {
        log_error("%s: invalid vhsot index %lu\n", __func__, index);
        return -1;
    }

    int client_socket_fd = vhost->clients->data[index];
    write(client_socket_fd, resp->res, resp->res_len);

    return 0;
}
