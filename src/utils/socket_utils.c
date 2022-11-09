#include "socket_utils.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCK_RD_BUFF_SIZE 256

char *read_from_connection(int socket_fd, size_t *data_len)
{
    size_t capacity = SOCK_RD_BUFF_SIZE;
    size_t size = 0;
    char *buffer = malloc(capacity);

    ssize_t num_read;
    while ((num_read = read(socket_fd, buffer + size, SOCK_RD_BUFF_SIZE - 1))
           > 0)
    {
        size += num_read;
        size_t remaining_capacity = capacity - size;
        if (remaining_capacity < SOCK_RD_BUFF_SIZE)
            buffer = realloc(buffer, (capacity += SOCK_RD_BUFF_SIZE));
    }
    if (num_read == -1)
    {
        // TODO: logging (error while reading)
        *data_len = 1;
        return NULL;
    }
    *data_len = size;

    return realloc(buffer, size);
}

/*
 * Returns success status
 */
bool set_socket_nonblocking_mode(int socket_fd)
{
    int flags;

    // First, retrieve the flags associated to the socket file descriptor
    if ((flags = fcntl(socket_fd, F_GETFL, 0)) == -1)
    {
        // TODO: looging (could not retrieve flags from socket fd)
        return false;
    }

    // Set to non-blocking mode
    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1)
    {
        // TODO: logging (could not set flag to non-blocking)
        return false;
    }

    return true;
}

char *get_local_ip_addr(void)
{
    // TODO :^)
    return "192.168.219.175";
}
