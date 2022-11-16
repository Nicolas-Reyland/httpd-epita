#include "write_response.h"

#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/sendfile.h>
#include <unistd.h>

#include "network/client.h"
#include "utils/logging.h"

int write_response(struct client *client, struct response *resp)
{
    if (client == NULL)
    {
        log_error("%s:  client is null\n", __func__);
        return -1;
    }

    log_debug("Writing to %d from %d with lengths %zu and %zu\n",
              client->socket_fd, resp->fd, resp->file_len, resp->res_len);

    // Write headers
    size_t total_num_written = 0;
    size_t num_written = 0;
    while (
        (num_written = write(client->socket_fd, resp->res + total_num_written,
                             resp->res_len - total_num_written))
            != 0
        && total_num_written < resp->res_len)
        total_num_written += num_written;

    // Write file (if needed)
    if (resp->fd != -1)
    {
        off_t offset = 0;
        ssize_t sfile_len = resp->file_len;
        while (offset < sfile_len)
        {
            if (sendfile(client->socket_fd, resp->fd, &offset, resp->file_len)
                == -1)
            {
                log_error("%s(sendfile %d %d %zu %zu): %s\n", __func__,
                          client->socket_fd, resp->fd, offset, resp->file_len,
                          strerror(errno));
                return -1;
            }
        }
    }

    return 0;
}
