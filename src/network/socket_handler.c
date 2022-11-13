#include "socket_handler.h"

#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "multithreading/write_response.h"
#include "network/client.h"
#include "network/vhost.h"
#include "response/response.h"
#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/parsers/http/parser_request.h"
#include "utils/socket_utils.h"
#include "utils/state.h"
#include "utils/vector_client/vector_client.h"

#define DEBUG_MAX_DATA_SIZE 300

void register_connection(int host_socket_fd)
{
    // get vhost associated to the socket fd
    struct vhost *vhost = vhost_from_host_socket(host_socket_fd);
    if (vhost == NULL)
    {
        log_error("Could not find host from socket_fd %d\n", host_socket_fd);
        return;
    }

    // There might be multiple new clients
    while (true)
    {
        struct sockaddr in_addr;
        socklen_t in_len = sizeof(in_addr);
        int client_socket_fd = accept(host_socket_fd, &in_addr, &in_len);

        if (client_socket_fd == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            else
            {
                log_error("Could not accept connection for host %s\n",
                          hash_map_get(vhost->map, "server_name"));
                break;
            }
        }

        char host_buffer[256], port_buffer[256];
        if (getnameinfo(&in_addr, in_len, host_buffer, sizeof(host_buffer),
                        port_buffer, sizeof(port_buffer),
                        NI_NUMERICHOST | NI_NUMERICSERV)
            == 0)
        {
            log_debug("Accepted connection on descriptor %d @ %s:%s\n",
                      client_socket_fd, host_buffer, port_buffer);
        }

        if (!set_socket_nonblocking_mode(client_socket_fd))
        {
            log_error("%s: Could not set socket to non-blocking mode\n",
                      __func__);
            CLOSE_ALL(client_socket_fd);
            continue;
        }

        struct epoll_event event = { 0 };
        event.data.fd = client_socket_fd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(g_state.env->epoll_fd, EPOLL_CTL_ADD, client_socket_fd,
                      &event)
            == -1)
        {
            CLOSE_ALL(client_socket_fd);
            log_error("%s: Could not register new client %d to epoll\n",
                      __func__, client_socket_fd);
            continue;
        }

        // register client socket fd to vhost
        char *client_ip_addr = g_state.logging ? strdup(host_buffer) : NULL;
        struct client *new_client =
            init_client(vhost, client_socket_fd, client_ip_addr);

        // lock clients vector
        {
            int error;
            if ((error = pthread_mutex_lock(&vhost->clients_mutex)))
            {
                log_error("[%d] %s(clients lock): %s\n", pthread_self(),
                          __func__, strerror(error));
                destroy_client(new_client, true);
                continue;
            }
            vector_client_append(vhost->clients, new_client);
            pthread_mutex_unlock(&vhost->clients_mutex);
        }
    }
}

struct vhost *vhost_from_host_socket(int socket_fd)
{
    for (size_t i = 0; i < g_state.env->config->num_vhosts; ++i)
        if (socket_fd == g_state.env->vhosts[i].socket_fd)
            return g_state.env->vhosts + i;

    return NULL;
}

/*
 * Locks the mutex on a return != NULL
 */
struct client *client_from_client_socket(int socket_fd, bool wait)
{
    for (size_t i = 0; i < g_state.env->config->num_vhosts; ++i)
    {
        struct client *client =
            vector_client_find(g_state.env->vhosts[i].clients, socket_fd, wait);
        if (client != NULL)
            return client;
    }

    return NULL;
}

void process_data(struct client *client, char *data, size_t size)
{
    if (client == NULL)
    {
        log_error("%s: Got null client\n", __func__);
        return;
    }

    if (data == NULL)
    {
        log_error("%s: Got empty data\n", __func__);
        return;
    }

    // Attention ! Does not print anything after the first 0 byte
    if (g_state.logging && size < DEBUG_MAX_DATA_SIZE)
        // Don't want to allocate this if we aren't debugging
        log_debug("Got: '''\n%s\n'''\n",
                  strncat(memset(alloca(size + 1), 0, 1), data, size));

    struct response *resp = parsing_http(data, size, client);
    write_response(client, resp);

    // Attention ! Does not print anything after the first 0 byte
    if (g_state.logging && resp->res_len < DEBUG_MAX_DATA_SIZE)
        // Don't want to allocate this if we aren't debugging
        log_debug("Got: '''\n%s\n'''\n",
                  strncat(memset(alloca(resp->res_len + 1), 0, 1), resp->res,
                          resp->res_len));
    free_response(resp);
}

ssize_t incoming_connection(int client_socket_fd)
{
    for (size_t i = 0; i < g_state.env->config->num_vhosts; ++i)
        if (client_socket_fd == g_state.env->vhosts[i].socket_fd)
            return i;

    return -1;
}

void close_connection(int client_socket_fd)
{
    log_message(LOG_STDOUT, "%s: Closing connection with %d\n", __func__,
                client_socket_fd);
    // Remove (deregister) the file descriptor
    epoll_ctl(g_state.env->epoll_fd, EPOLL_CTL_DEL, client_socket_fd, NULL);
    // close the connection on our end, too
    close(client_socket_fd);
    // remove link between vhost and client
    struct client *client = client_from_client_socket(client_socket_fd, false);
    if (client == NULL)
    {
        log_error("%s: Could not find host associated to socket %d\n", __func__,
                  client_socket_fd);
        return;
    }
    vector_client_remove(client);
    // Unlock mutex
    pthread_mutex_unlock(&client->mutex);
}
