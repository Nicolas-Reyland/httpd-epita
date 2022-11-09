#include "handler.h"

#include <errno.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "utils/socket_utils.h"

void register_connection(struct server_env *env)
{
    int host_socket_fd = env->server_socket_fd;

    // There might be multiple new clients
    while (true)
    {
        struct sockaddr in_addr;
        socklen_t in_len = sizeof(in_addr);
        int client_socket_fd = accept(host_socket_fd, &in_addr, &in_len);

        if (client_socket_fd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // TODO: logging (processed all incoming connections);
                break;
            }
            else
            {
                // TODO: logging (error)
                break;
            }
        }

#if CUSTOM_DEBUG
        char host_buffer[NI_MAXHOST], port_buffer[NI_MAXSERV];
        if (getnameinfo(&in_addr, in_len, host_buffer, sizeof(host_buffer),
                        port_buffer, sizeof(port_buffer),
                        NI_NUMERICHOST | NI_NUMERICSERV)
            == 0)
        {
            printf("Accepted connection on descriptor %d "
                   "(host=%s, port=%s)\n",
                   client_socket_fd, host_buffer, port_buffer);
        }
#endif

        if (!set_socket_nonblocking_mode(client_socket_fd))
        {
            // TODO: logging
            continue;
        }

        struct epoll_event event = { 0 };
        event.data.fd = client_socket_fd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(env->epoll_fd, EPOLL_CTL_ADD, client_socket_fd, &event)
            == -1)
        {
            // TODO: logging (errno)
            continue;
        }
    }
}

void process_data(struct server_env *env, int event_index, char *data,
                  size_t size)
{
    (void)env;
    (void)event_index;
    (void)data;
    (void)size;
}
