#include "socket_utils.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "utils/logging.h"

#define SOCK_RD_BUFF_SIZE 256

char *read_from_connection(int socket_fd, size_t *data_len, bool *alive)
{
    *alive = true;
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
    if (num_read == 0)
    {
        free(buffer);
        *alive = false;
        *data_len = 0;
        return NULL;
    }
    if (num_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK)
    {
        free(buffer);
        log_error("%s(read data): %s\n", __func__, strerror(errno));
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
        log_error("%s: could not retrieve flags from socket fd\n");
        return false;
    }

    // Set to non-blocking mode
    flags |= O_NONBLOCK;
    if (fcntl(socket_fd, F_SETFL, flags) == -1)
    {
        log_error("%s(fcntl set nonblocking flag): %s", __func__,
                  strerror(errno));
        return false;
    }

    return true;
}

ssize_t safe_write(int fd, void *buf, size_t len)
{
    sigset_t oldset, newset;
    ssize_t result;
    siginfo_t si;
    struct timespec ts = { 0 };

    sigemptyset(&newset);
    sigaddset(&newset, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &newset, &oldset);

    result = write(fd, buf, len);

    while (sigtimedwait(&newset, &si, &ts) >= 0 || errno != EAGAIN)
        continue;
    pthread_sigmask(SIG_SETMASK, &oldset, 0);

    return result;
}
