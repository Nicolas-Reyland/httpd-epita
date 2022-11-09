#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <stdbool.h>
#include <stddef.h>

char *read_from_connection(int socket_fd, size_t *data_len);

bool set_socket_nonblocking_mode(int socket_fd);

char *get_local_ip_addr(void);

#endif /* !SOCKET_UTILS_H */
