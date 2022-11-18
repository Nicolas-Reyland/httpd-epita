#include "vector_client.h"

#include <stdlib.h>
#include <string.h>

#include "utils/logging.h"
#include "utils/mem.h"
#include "utils/state.h"

static struct vector_client *vector_client_resize(struct vector_client *v,
                                                  size_t n);

/*
** Initialize the vector_client with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_init(size_t n)
{
    struct client **data = calloc(n, sizeof(struct client *));
    if (data == NULL)
    {
        log_error("%s: Out of memory (data: %zu)\n", __func__, n);
        return NULL;
    }
    struct vector_client *v = malloc(sizeof(struct vector_client));
    if (v == NULL)
    {
        free(data);
        log_error("%s: Out of memory (struct)\n", __func__);
        return NULL;
    }

    v->size = 0;
    v->capacity = n;
    v->data = data;

    return v;
}

/*
** Release the memory used by the vector_client.
** Does nothing if `v` is `NULL`.
*/
void destroy_vector_client(struct vector_client *v)
{
    if (v == NULL)
        return;
    for (size_t i = 0; i < v->size; ++i)
        destroy_client(v->data[i], true);
    FREE_SET_NULL(v->data);
    FREE_SET_NULL(v);
}

/*
 * Returns the client which has the socket_fd file descriptor
 * NULL if it is not found
 */
struct client *vector_client_find(struct vector_client *v, int socket_fd)
{
    for (size_t i = 0; i < v->size; ++i)
    {
        if (v->data[i] != NULL)
        {
            struct client *client = v->data[i];
            if (client->socket_fd == socket_fd)
                return client;
        }
    }

    return NULL;
}

/* ** Resize the vector_client to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_resize(struct vector_client *v, size_t n)
{
    if (v == NULL)
    {
        log_warn("[%u] %s: v is NULL in resize\n", pthread_self(), __func__);
        return NULL;
    }

    v->data = realloc(v->data, (v->capacity = n) * sizeof(struct client *));
    if (v->data == NULL)
    {
        log_error("[%u] %s: Out of memory\n", pthread_self(), __func__);
        return NULL;
    }

    if (v->capacity < v->size)
        v->size = v->capacity;

    return v;
}

/*
 * Returns validitity status of 'index' in 'v'
 */
bool vector_client_valid_index(struct vector_client *v, ssize_t index)
{
    size_t index_t = index;
    return v == NULL ? false : index >= 0 && index_t < v->size;
}

/*
** Append an element to the end of the vector_client. Expand the vector_client
** if needed.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_append(struct vector_client *v,
                                           struct client *client)
{
    log_debug("[%u] %s: Adding client %d to v\n", pthread_self(), __func__,
              client->socket_fd);

    if (v == NULL)
    {
        log_error("[%u] %s: vector is NULL\n", pthread_self(), __func__);
        destroy_client(client, true);
        return NULL;
    }

    if (v->size == v->capacity)
        if ((v = vector_client_resize(v, 2 * v->capacity)) == NULL)
            return NULL;

    v->data[(client->index = v->size++)] = client;
    return v;
}

/*
** Remove the element at the index `i`.
** Replace it with the last element.
** Returns `NULL` if an error occured.
**
** The vector and client MUST be locked
**
*/
struct vector_client *vector_client_remove(struct client *client)
{
    if (client == NULL || client->vhost == NULL)
        return NULL;

    struct vector_client *v = client->vhost->clients;

    ssize_t index = client->index;
    if (v == NULL || !vector_client_valid_index(v, index))
        return NULL;
    size_t uindex = client->index;

    v->data[index] = NULL;
    destroy_client(client, true);

    // Replace
    size_t last_index = v->size - 1;
    if (uindex != last_index && v->data[last_index] != NULL)
    {
        // Redo the read of the last client (it is locked now)
        v->data[index] = v->data[last_index];
        v->data[index]->index = index;
    }
    --v->size;

    size_t half_cap = v->capacity / 2;
    v = v->size < half_cap ? vector_client_resize(v, half_cap) : v;

    return v;
}
