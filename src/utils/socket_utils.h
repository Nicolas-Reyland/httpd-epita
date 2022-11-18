#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

char *read_from_connection(int socket_fd, size_t *data_len, bool *alive);

bool set_socket_nonblocking_mode(int socket_fd);

/*
 * Wrapper around the write syscall that is not is SIGPIPE-safe
 * It is also thread-safe.
 */
ssize_t safe_write(int fd, void *buf, size_t len);

#endif /* !SOCKET_UTILS_H */
