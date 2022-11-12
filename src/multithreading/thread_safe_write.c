#include "thread_safe_write.h"

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/sendfile.h>
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
    log_debug("Writing %d to %d with lengths %zu and %zu\n", client_socket_fd,
              resp->fd, resp->file_len, resp->res_len);

    // Write headers
    size_t total_num_written = 0;
    size_t num_written = 0;
    while ((num_written = write(client_socket_fd, resp->res + total_num_written,
                                resp->res_len - total_num_written))
               != 0
           && total_num_written < resp->res_len)
        total_num_written += num_written;

    log_debug("Header written\n");

    // Write file (if needed)
    if (resp->fd != -1)
    {
        off_t offset = 0;
        ssize_t sfile_len = resp->file_len;
        while (offset != sfile_len)
        {
            if (sendfile(client_socket_fd, resp->fd, &offset, resp->file_len)
                == -1)
            {
                log_error("%s: %s\n", __func__, strerror(errno));
                return -1;
            }
        }
        log_debug("%zd bytes written\n", offset);
    }
    log_debug("File written\n");

    return 0;
}
