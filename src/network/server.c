#include "server.h"

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

#include "handler.h"
#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/socket_utils.h"

// this is just a btach size for events ...
// not the max number of sockets in the epoll
#define EPOLL_MAXEVENTS 64

static int create_and_bind(char *ip_addr, char *port);

static struct server_env *setup_server(int num_threads,
                                       struct server_config *config);

_Noreturn void start_all(int num_threads, struct server_config *config)
{
    struct server_env *env = setup_server(num_threads, config);
    if (env == NULL)
    {
        // TODO: logging (some error occured, idk man)
        free_server_config(config, true);
        exit(EXIT_FAILURE);
    }

    // Everything is up and ready ! Get it running !!!
    run_server(env);
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
                // logging (errno ?)
                log_error("Error occured in epoll: %d\n", env->events[i]);
                close(env->events[i].data.fd);
                // Set things to zero/-1 no ?
                // env->events[i].data.fd = -1;
                continue;
            }
            // File not available for reading, so just skipping it
            if (!(env->events[i].events & EPOLLIN))
            {
                // maybe log to see why we were notified ?
                continue;
            }
            // Event on the server socket: means there is one more client !
            else if (env->server_socket_fd == env->events[i].data.fd)
            {
                // Maybe log new client connection ?
                register_connection(env);
                // TODO: should we read the data or something ?
            }
            // There is data to be read
            else
            {
                size_t data_len;
                char *data =
                    read_from_connection(env->events[i].data.fd, &data_len);
                // when threading, add (i, data, size) to queue instead of
                // doing it now, in the main loop
                process_data(env, i, data, data_len);
            }
        }
    }

    // clean up
    free_server_env(env, true, true);

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
        // logging (invalid ip adddress)
        log_error("Could not retrieve ip address from string '%s'\n", ip_addr);
        return -1;
    }
    void *addr_ptr = &addr_in;
    hints.ai_addr = addr_ptr;
    hints.ai_addrlen = sizeof(addr_in);

    int gai_err_code;
    if ((gai_err_code = getaddrinfo(NULL, port, &hints, &result)) != 0)
    {
        // looging with gai_strerror(gai_err_code)
        log_error("Could not get address info from port '%s'\n", port);
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
        log_error("No suitable address found\n");
        // logging (no suitable address)
        return -1;
    }

    // clean up
    freeaddrinfo(result);

    return socket_fd;
}

static int setup_socket(int epoll_fd, char *ip_addr, char *port, bool is_vhost);

/*
 * Setup the following things :
 *  - server & vhosts socket file descriptors
 *  - server & vhosts socket file descriptors modes (nonblocking)
 *  - epoll file descriptor
 *  - epoll events buffer (see EPOLL_MAXEVENTS)
 *
 *  Return NULL on failure.
 */
struct server_env *setup_server(int num_threads, struct server_config *config)
{
    if (config == NULL)
        return NULL;

    struct server_env *env = malloc(sizeof(struct server_env));
    int *vhosts_socket_fds = calloc(config->num_vhosts, sizeof(int));
    struct epoll_event *events =
        calloc(EPOLL_MAXEVENTS, sizeof(struct epoll_event));

    // check allocations
    if (env == NULL || vhosts_socket_fds || events == NULL)
    {
        free(env);
        free(vhosts_socket_fds);
        free(events);
        log_error("Out of memory (%s)\n", __func__);
        return NULL;
    }

    // set up the epoll instance
    int epoll_fd = epoll_create(0);
    if (epoll_fd == -1)
    {
        free(env);
        free(events);
        // TODO: logging (errno ?)
        return NULL;
    }

    // set up main server
    char *local_ip_addr = get_local_ip_addr();
    int server_socket_fd =
        setup_socket(epoll_fd, local_ip_addr, HTTP_PORT, false);
    free(local_ip_addr);
    if (server_socket_fd == -1)
    {
        // TODO: logging (failed setting up global)
        FREE_SET_NULL(env, vhosts_socket_fds, events)
        return NULL;
    }

    // set up vhosts
    for (size_t i = 0; i < config->num_vhosts; ++i)
    {
        char *vhost_ip_addr = hash_map_get(config->vhosts[i], "ip");
        char *vhost_port = hash_map_get(config->vhosts[i], "port");
        vhosts_socket_fds[i] =
            setup_socket(epoll_fd, vhost_ip_addr, vhost_port, true);

        if (vhosts_socket_fds[i] == -1)
        {
            // TODO: logging (failed setting up vhost)

            // close all the previously opened sockets
            close(server_socket_fd);
            for (size_t j = 0; j < i; ++j)
                close(vhosts_socket_fds[j]);

            FREE_SET_NULL(env, vhosts_socket_fds, events)
            return NULL;
        }
    }

    env->config = config;
    env->server_socket_fd = server_socket_fd;
    env->vhosts_socket_fds = vhosts_socket_fds;
    env->epoll_fd = epoll_fd;
    env->events = events;
    env->num_threads = num_threads;

    return env;
}

int setup_socket(int epoll_fd, char *ip_addr, char *port, bool is_vhost)
{
    if (ip_addr == NULL || port == NULL)
        return -1;

    (void)is_vhost;
    // First, get a socket for this config
    int socket_fd = create_and_bind(ip_addr, port);
    if (socket_fd == -1)
    {
        // TODO: logging (could not create socket or smthin)
        return -1;
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
        close(socket_fd);
        // TODO: logging (could not set socket to non-blocking mode)
        return -1;
    }

    if (listen(socket_fd, SOMAXCONN) == -1)
    {
        close(socket_fd);
        // TODO: logging (errno)
        return -1;
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
        close(socket_fd);
        // TODO: logging (errno?)
        return -1;
    }

    return socket_fd;
}
