#include "server.h"

#include <arpa/inet.h>
#include <err.h>
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

#include "network/socket_handler.h"
#include "network/vhost.h"
#include "process/sig_handlers.h"
#include "process/signals.h"
#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/socket_utils.h"
#include "utils/state.h"

// this is just a btach size for events ...
// not the max number of sockets in the epoll
#define EPOLL_MAXEVENTS 64

static int write_process_pid(char *pid_file);

static struct server_env *setup_server(struct server_config *config);

_Noreturn void start_all(struct server_config *config, char *pid_file)
{
    log_info("Starting server [%d]\n", getpid());
    if (write_process_pid(pid_file) == -1)
    {
        log_error("Could not write process pid to file at '%s'\n", pid_file);
        exit(EXIT_FAILURE);
    }

    // Setup the signal handlers
    if (setup_signal_handlers() == -1)
        graceful_shutdown();

    set_g_state_logging(config);
    struct server_env *env = setup_server(config);
    if (env == NULL)
    {
        log_error("Could not setup the server. Exiting.\n");
        free_server_config(config, true);
        exit(EXIT_FAILURE);
    }
    log_message(LOG_STDOUT | LOG_INFO, "Setup done. Starting server.\n");

    // Setup all the global variables
    if (setup_g_state(env) == -1)
        graceful_shutdown();

    // Everything is up and ready ! Get it running !!!
    run_server(env);
}

/*
 * Returns -1 on failure, 0 on succes and -2 if pid_file is NULL
 */
int write_process_pid(char *pid_file)
{
    if (pid_file == NULL)
        return -2;

    FILE *file = fopen(pid_file, "w");
    if (file == NULL)
        return -1;

    pid_t pid = getpid();
    char pid_s[8]; // Max pid value is around 4 million for 64-bits platorms, so
                   // 8 chars suffice !
    sprintf(pid_s, "%d", pid);
    size_t pid_s_len = strlen(pid_s);

    size_t total_num_written = 0;
    size_t num_written = 0;
    while ((num_written = fwrite(pid_s + total_num_written, 1,
                                 pid_s_len - total_num_written, file))
               != 0
           && total_num_written < pid_s_len)
        total_num_written += num_written;

    fclose(file);
    if (total_num_written < pid_s_len)
    {
        log_error("%s: Could not write whole pid to %s : %zu out of %zu\n",
                  __func__, pid_file, total_num_written, pid_s_len);
        return -1;
    }

    return 0;
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
            int socket_fd = env->events[i].data.fd;
            if (env->events[i].events & (EPOLLHUP | EPOLLERR))
            {
                // logging (errno ?)
                log_error("Error occured in epoll: %d\n", env->events[i]);
                close_connection(env, socket_fd);
                continue;
            }
            // File not available for reading, so just skipping it
            if (!(env->events[i].events & EPOLLIN))
            {
                // maybe log to see why we were notified ?
                continue;
            }
            // Event on the server socket: means there is one more client !
            else if (incoming_connection(env, socket_fd))
            {
                // Maybe log new client connection ?
                register_connection(env, socket_fd);
            }
            // There is data to be read
            else
            {
                bool alive;
                size_t data_len;
                char *data = read_from_connection(socket_fd, &data_len, &alive);
                if (!alive)
                {
                    close_connection(env, socket_fd);
                    continue;
                }
                // when threading, add (i, data, size) to queue instead of
                // doing it now, in the main loop
                process_data(env, i, data, data_len);
                free(data);
            }
        }
    }

    // clean up
    free_server_env(env, true, true);

    exit(EXIT_SUCCESS);
}

/*
 * Setup the following things :
 *  - vhosts socket file descriptors
 *  - vhosts socket file descriptors modes (nonblocking)
 *  - vhosts client fds
 *  - epoll file descriptor
 *  - epoll events buffer (see EPOLL_MAXEVENTS)
 *
 *  Return NULL on failure.
 */
struct server_env *setup_server(struct server_config *config)
{
    if (config == NULL)
        return NULL;

    struct server_env *env = malloc(sizeof(struct server_env));
    struct vhost *vhosts = init_vhosts(config);
    struct epoll_event *events =
        calloc(EPOLL_MAXEVENTS, sizeof(struct epoll_event));

    // check allocations
    if (env == NULL || vhosts == NULL || events == NULL)
    {
        FREE_SET_NULL(env, vhosts, events);
        log_error("Out of memory (%s)\n", __func__);
        return NULL;
    }

    // set up the epoll instance
    int epoll_fd = epoll_create(1);
    if (epoll_fd == -1)
    {
        FREE_SET_NULL(env, events);
        log_error("Could not create epoll file descriptor\n");
        return NULL;
    }

    // set up vhosts
    for (size_t i = 0; i < config->num_vhosts; ++i)
    {
        char *vhost_ip_addr = hash_map_get(config->vhosts[i], "ip");
        char *vhost_port = hash_map_get(config->vhosts[i], "port");
        log_message(LOG_STDOUT | LOG_DEBUG, "Adding vhost @ %s:%s\n",
                    vhost_ip_addr, vhost_port);
        vhosts[i].socket_fd = setup_socket(epoll_fd, vhost_ip_addr, vhost_port);

        if (vhosts[i].socket_fd == -1)
        {
            log_error("Could not setup the vhost server n%zu\n", i + 1);

            // close all the previously opened sockets
            for (size_t j = 0; j < i; ++j)
                free_vhost(vhosts + j, false, false);

            FREE_SET_NULL(env, events)
            return NULL;
        }
    }

    env->config = config;
    env->vhosts = vhosts;
    env->epoll_fd = epoll_fd;
    env->events = events;

    return env;
}

static int create_socket(char *ip_addr, char *port);

int setup_socket(int epoll_fd, char *ip_addr, char *port)
{
    if (ip_addr == NULL || port == NULL)
        return -1;

    // First, get a socket for this config
    int socket_fd = create_socket(ip_addr, port);
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

int create_socket(char *ip_addr, char *port)
{
    struct addrinfo hints = { 0 }; // init all fields to zero
    hints.ai_flags = AI_PASSIVE; // All interfaces
    hints.ai_family = AF_INET; // IPv4 choices
    hints.ai_protocol = IPPROTO_TCP; // TCP Protocol only
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    // IP addr
    struct sockaddr_in addr_in = { 0 };
    if (!inet_aton(ip_addr, &addr_in.sin_addr))
    {
        // logging (invalid ip adddress)
        log_error("Could not retrieve ip address from string '%s'\n", ip_addr);
        return -1;
    }
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(strtol(port, NULL, 10));
    void *addr_ptr = &addr_in;
    hints.ai_addr = addr_ptr;
    hints.ai_addrlen = sizeof(struct sockaddr_in);

    int gai_err_code;
    struct addrinfo *result;
    if ((gai_err_code = getaddrinfo(NULL, port, &hints, &result)) != 0)
    {
        log_error(
            "Could not get address info from port '%s' with error: ''%s'\n",
            port, gai_strerror(gai_err_code));
        freeaddrinfo(result);
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

        // TODO: only for vhost, or also for the global server ?
        int override = 1;
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &override,
                       sizeof(int))
            == -1)
            continue;

        if (bind(socket_fd, addr_ptr, sizeof(struct sockaddr_in)) == 0)
            // Successful bind
            break;

        // Could not bind, so closing it
        close(socket_fd);
    }

    // clean up
    freeaddrinfo(result);

    if (addr == NULL)
    {
        log_error("No suitable address found\n");
        if (errno != 0)
            warn(__func__);
        // logging (no suitable address)
        return -1;
    }

    return socket_fd;
}
