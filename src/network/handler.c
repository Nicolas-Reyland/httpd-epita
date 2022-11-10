#include "handler.h"

#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utils/logging.h"
#include "utils/socket_utils.h"

void register_connection(struct server_env *env, int host_socket_fd)
{
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
        char host_buffer[256], port_buffer[256];
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
    if (data == NULL)
    {
        log_error("%s: Got empty data\n", __func__);
        return;
    }

    printf("Got: '''\n");
    for (size_t i = 0; i < size; ++i)
        printf("%c", data[i]);
    printf("\n'''\n");

    const char reply[] = "HTTP/1.0 200 OK\r\n"
                         "Connection: close\r\n"
                         "Content-Length: 21\r\n"
                         "\r\n"
                         "this is the body !!!\n";

    int client_socket_fd = env->events[event_index].data.fd;
    size_t reply_size = sizeof(reply) - 1;
    write(client_socket_fd, reply, reply_size);
}

bool incoming_connection(struct server_env *env, int client_socket_fd)
{
    for (size_t i = 0; i < env->config->num_vhosts; ++i)
        if (client_socket_fd == env->vhosts_socket_fds[i])
            return true;

    return false;
}

void close_connection(struct server_env *env, int client_socket_fd)
{
    log_message(LOG_STDOUT, "%s: Closing connection with %d\n", __func__,
                client_socket_fd);
    // Remove (deregister) the file descriptor
    epoll_ctl(env->epoll_fd, EPOLL_CTL_DEL, client_socket_fd, NULL);
    // close the connection on our end, too
    close(client_socket_fd);
}
