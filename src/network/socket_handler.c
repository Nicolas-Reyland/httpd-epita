#include "socket_handler.h"

#include <arpa/inet.h>
#include <errno.h>
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
#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/parsers/http/parser_request.h"
#include "utils/response/response.h"
#include "utils/socket_utils.h"
#include "utils/state.h"
#include "utils/vector_client/vector_client.h"

#define DEBUG_MAX_DATA_SIZE 300

static void close_connection_client(struct client *client);

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
        struct sockaddr_in in_addr;
        socklen_t in_len = sizeof(in_addr);
        void *in_addr_ptr = &in_addr;
        int c_socket_fd = accept(host_socket_fd, in_addr_ptr, &in_len);

        if (c_socket_fd == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                log_error("%s(accept): %s: %s\n", __func__, strerror(errno));
            break;
        }

        char host[INET_ADDRSTRLEN + 1];
        if (inet_ntop(AF_INET, &in_addr.sin_addr, host, INET_ADDRSTRLEN)
            != NULL)
        {
            log_info("Accepted connection on %d @ %s\n", c_socket_fd, host);
        }

        if (!set_socket_nonblocking_mode(c_socket_fd))
        {
            log_error("%s: Could not set socket to non-blocking mode\n",
                      __func__);
            close(c_socket_fd);
            continue;
        }

        struct epoll_event event = { 0 };
        event.data.fd = c_socket_fd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(g_state.env->epoll_fd, EPOLL_CTL_ADD, c_socket_fd, &event)
            == -1)
        {
            close(c_socket_fd);
            log_error("%s: Could not register new client %d to epoll\n",
                      __func__, c_socket_fd);
            continue;
        }

        // register client socket fd to vhost
        struct client *client = init_client(vhost, c_socket_fd, strdup(host));
        if (client == NULL)
        {
            close(c_socket_fd);
            continue;
        }

        vector_client_append(vhost->clients, client);
        log_debug("Appended %d to clients\n", c_socket_fd);
    }
}

static int process_data(struct client *client, char *data, size_t size);

void handle_incoming_data(int socket_fd)
{
    struct client *client = client_from_client_socket(socket_fd);
    if (client == NULL)
    {
        log_error("%s: Could not find client associated to %d\n", __func__,
                  socket_fd);
        return;
    }
    bool alive;
    size_t data_len;
    char *data = read_from_connection(socket_fd, &data_len, &alive);
    if (!alive)
    {
        close_connection(socket_fd);
        return;
    }
    // add data to buffer
    size_t new_buffered_size = client->buffered_size + data_len;
    if (client->buffered_data == NULL)
    {
        client->buffered_data = data;
        client->buffered_size = data_len;
    }
    else
    {
        client->buffered_data =
            realloc(client->buffered_data, new_buffered_size);
        memcpy(client->buffered_data + client->buffered_size, data, data_len);
        client->buffered_size = new_buffered_size;
        free(data);
    }

    // process data. returns 0 if we didn't read enough data
    if (!process_data(client, client->buffered_data, client->buffered_size))
        log_debug("Did not receive everything from %d. Waiting for more\n",
                  client->socket_fd);
}

struct vhost *vhost_from_host_socket(int socket_fd)
{
    for (size_t i = 0; i < g_state.env->config->num_vhosts; ++i)
        if (socket_fd == g_state.env->vhosts[i].socket_fd)
            return g_state.env->vhosts + i;

    return NULL;
}

struct client *client_from_client_socket(int socket_fd)
{
    for (size_t i = 0; i < g_state.env->config->num_vhosts; ++i)
    {
        struct client *client =
            vector_client_find(g_state.env->vhosts[i].clients, socket_fd);

        if (client != NULL)
            return client;
    }

    return NULL;
}

/*
 * Returns 1 if the connection was closed. 0 if we need more data.
 * Returns -1 on error
 */
int process_data(struct client *client, char *data, size_t size)
{
    if (client == NULL)
    {
        log_error("%s: Got null client\n", __func__);
        return -1;
    }

    if (data == NULL)
    {
        log_error("%s: Got empty data\n", __func__);
        return -1;
    }

    log_info("[%u] Processing data for %d\n", pthread_self(),
             client->socket_fd);

#ifdef LOG_MSG
    // Attention ! Does not print anything after the first 0 byte
    if (g_state.logging && size < DEBUG_MAX_DATA_SIZE)
        // Don't want to allocate this if we aren't debugging
        log_debug("Got: '''\n%s\n'''\n",
                  strncat(memset(alloca(size + 1), 0, 1), data, size));
#endif /* LOG_MSG */

    struct response *resp = parsing_http(data, size, client);
    write_response(client, resp);

#ifdef LOG_MSG
    // Attention ! Does not print anything after the first 0 byte
    if (g_state.logging && resp->res_len < DEBUG_MAX_DATA_SIZE)
        // Don't want to allocate this if we aren't debugging
        log_debug("Got: '''\n%s\n'''\n",
                  strncat(memset(alloca(resp->res_len + 1), 0, 1), resp->res,
                          resp->res_len));
#endif /* LOG_MSG */
    bool close_connection = resp->close_connection;
    free_response(resp);

    if (close_connection)
    {
        log_debug("%s: response object asks to close connection %d\n", __func__,
                  client->socket_fd);
        close_connection_client(client);

        return 1;
    }

    return 0;
}

ssize_t incoming_connection(int client_socket_fd)
{
    for (size_t i = 0; i < g_state.env->config->num_vhosts; ++i)
        if (client_socket_fd == g_state.env->vhosts[i].socket_fd)
            return i;

    return -1;
}

/*
 * The client and the vector MUST be locked
 */
void close_connection(int socket_fd)
{
    struct client *client = client_from_client_socket(socket_fd);
    log_info("[%u] Closing connection with %d\n", pthread_self(),
             client->socket_fd);
    close_connection_client(client);
}

void close_connection_client(struct client *client)
{
    // Remove (deregister) the file descriptor
    epoll_ctl(g_state.env->epoll_fd, EPOLL_CTL_DEL, client->socket_fd, NULL);
    // destroy the client (closing file descriptors in the process)
    vector_client_remove(client);
}
