#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "server_env.h"
#include "utils/hash_map/hash_map.h"
#include "utils/parsers/config/config_parser.h"

#define EPOLL_MAXEVENTS 32

static int create_and_bind(char *ip_addr, char *port);

static bool set_socket_nonblocking_mode(int socket_fd);

static struct server_env *setup_server(int num_threads,
                                       struct hash_map *config);

_Noreturn void start_all(struct server_config *config)
{
    (void)config;
    exit(EXIT_SUCCESS);
}

_Noreturn void run_server(struct server_env *env)
{
    // This loop has no break, (maybe one to come ?)
    while (true)
    {
        int num_events =
            epoll_wait(env->epoll_fd, env->events, EPOLL_MAXEVENTS, -1);
        for (int i = 0; i < num_events; ++i)
        {
            // Error has occured on file descriptor, or client closed connection
            if (env->events[i].events & (EPOLLHUP | EPOLLERR))
            {
                // TODO: logging (errno ?)
                close(env->events[i].data.fd);
                // Set things to zero/-1 no ?
                // env->events[i].data.fd = -1;
            }
            // File not available for reading, so just skipping it
            if (!(env->events[i].events & EPOLLIN))
            {
                // maybe log to see why we were notified ?
                continue;
            }
            // Event on the server socket: means there is one more client !
            else if (env->socket_fd == env->events[i].data.fd)
            {
                // Maybe log new client connection ?
                add_new_client(env);
            }
            // There is data to be read
            else
            {
                size_t size;
                char *data =
                    read_from_connection(env->events[i].data.fd, &size);
                // when threading, add (i, data, size) to queue instead of
                // doing it now, in the main loop
                handle_incoming_data(env, i, data, size);
            }
        }
    }

    // clean up
    free(env->events);
    close(env->socket_fd);

    exit(EXIT_SUCCESS);
}

int create_and_bind(char *ip_addr, char *port)
{
    struct addrinfo *result;

    struct addrinfo hints = { 0 }; // init all fields to zero
    hints.ai_flags = AI_PASSIVE; // All interfaces
    hints.ai_family = AF_INET; // IPv4 choices
    hints.ai_protocol = 0; // Any protocol, but may want to restrict to http ?
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    // IP addr
    struct sockaddr_in addr_in = { 0 };
    if (!inet_aton(ip_addr, &addr_in.sin_addr))
    {
        // TODO: logging (invalid ip adddress)
        return -1;
    }
    void *addr_ptr = &addr_in;
    hints.ai_addr = addr_ptr;
    hints.ai_addrlen = sizeof(addr_in);

    int gai_err_code;
    if ((gai_err_code = getaddrinfo(NULL, port, &hints, &result)) != 0)
    {
        // TODO: looging with gai_strerror(gai_err_code)
        return -1;
    }

    int socket_fd;
    struct addrinfo *addr = result;
    // Test all the potenntial addresses
    for (; addr != NULL; addr = addr->ai_next)
    {
        if ((socket_fd =
                 socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol))
            == -1)
            continue;

        if (bind(socket_fd, addr->ai_addr, addr->ai_addrlen) == 0)
            // Successful bind
            break;

        // Could not bind, so closing it
        close(socket_fd);
    }

    if (addr == NULL)
    {
        // TODO: logging (no suitable address)
        return -1;
    }

    // clean up
    freeaddrinfo(result);

    return socket_fd;
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

/*
 * Setup the following things :
 *  - socket file descriptor
 *  - socket file descriptor mode (nonblocking)
 *  - epoll file descriptor
 *  - epoll events buffer (see EPOLL_MAXEVENTS)
 *  - epoll event types
 *
 *  Return NULL on failure.
 */
struct server_env *setup_server(int num_threads, struct hash_map *config)
{
    struct server_env *env = malloc(sizeof(struct server_env));
    if (env == NULL)
    {
        // TODO: logging (OOM)
        return NULL;
    }

    // Buffer for epoll events
    struct epoll_event *events =
        calloc(EPOLL_MAXEVENTS, sizeof(struct epoll_event));
    if (events == NULL)
    {
        free(env);
        // TODO: logging (OOM)
        return NULL;
    }

    // First, get a socket for this config
    int socket_fd = create_and_bind(hash_map_get(config, "ip"),
                                    hash_map_get(config, "port"));
    if (socket_fd == -1)
    {
        free(env);
        free(events);
        // TODO: logging (could not create socket or smthin)
        return NULL;
    }

    /*
     * From epoll(7) man page :
     *
     * An application that employs the EPOLLET flag should use nonblocking
     * file descriptors to avoid having a blocking read or write starve a task
     * that is handling  multiple  file  descriptors.
     */
    if (!set_socket_nonblocking_mode(socket_fd))
    {
        free(env);
        free(events);
        close(socket_fd);
        // TODO: logging (could not set socket to non-blocking mode)
        return NULL;
    }

    if (listen(socket_fd, SOMAXCONN) == -1)
    {
        free(env);
        free(events);
        close(socket_fd);
        // TODO: logging (errno)
        return NULL;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        free(env);
        free(events);
        close(socket_fd);
        // TODO: logging (errno ?)
        return NULL;
    }

    struct epoll_event event = { 0 };
    event.data.fd = socket_fd;
    /*
     * EPOLLIN: get events when the fd is available for reading
     *
     * EPOLLET: so-called edge-triggered notifications
     * We have to make sure that we consume all the data when we get an
     * event from epoll because of this
     *
     */
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &event) == -1)
    {
        free(env);
        free(events);
        close(socket_fd);
        close(epoll_fd);
        // TODO: logging (errno?)
        return NULL;
    }

    env->config = config;
    env->socket_fd = socket_fd;
    env->epoll_fd = epoll_fd;
    env->event = event;
    env->events = events;
    env->num_threads = num_threads;

    return env;
}
