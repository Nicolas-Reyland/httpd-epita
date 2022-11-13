#include "vector_client.h"

#include <stdlib.h>
#include <string.h>

#include "utils/logging.h"
#include "utils/mem.h"

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
    FREE_SET_NULL(v->data, v);
}

/*
 * Returns the client which has the socket_fd file descriptor
 * NULL if it is not found
 *
 * 'wait' refers to the mutex locking function to use (lock or trylock)
 *
 * Attention !!
 * This function locks the client, returns it and does NOT unlock it
 * That is the calling function's job !
 *
 */
struct client *vector_client_find(struct vector_client *v, int socket_fd,
                                  bool wait)
{
    int (*lock_function)(pthread_mutex_t *) = NULL;
    if (wait)
        lock_function = pthread_mutex_lock;
    else
        lock_function = pthread_mutex_trylock;

    for (size_t i = 0; i < v->size; ++i)
    {
        if (v->data[i] != NULL && (*lock_function)(&v->data[i]->mutex) == 0)
        {
            struct client *client = v->data[i];
            if (client->socket_fd == socket_fd)
                return client;
            pthread_mutex_unlock(&client->mutex);
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
        return NULL;

    v->data = realloc(v->data, (v->capacity = n) * sizeof(struct client *));
    if (v->data == NULL)
        return NULL;

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
*if needed.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_append(struct vector_client *v,
                                           struct client *client)
{
    if (v == NULL)
    {
        destroy_client(client, true);
        return NULL;
    }

    if (v->size == v->capacity)
    {
        v = vector_client_resize(v, 2 * v->capacity);
        if (v == NULL)
            return NULL;
    }

    v->data[(client->index = v->size++)] = client;
    return v;
}

/*
** Remove the element at the index `i`.
** Replace it with the last element.
** Returns `NULL` if an error occured.
**
** The client MUST be locked
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

    // TODO: lock the vector (we are modifying values)

    /*
     * First set the client to null.
     * This is because in the destroy_client call, the mutex
     * associated to the client is first unlocked, then
     * destroyed. To prevent a thread to lock the client between
     * these two operations, we must first set it to null in the vector
     */
    v->data[index] = NULL;
    destroy_client(client, true);

    // Replace
    if (uindex != v->size - 1 && pthread_mutex_lock(&v->data[v->size - 1]->mutex) == 0)
    {
        // Redo the read of the last client
        v->data[index] = v->data[v->size - 1];
        --v->size;
        pthread_mutex_unlock(&v->data[index]->mutex);
    }

    size_t half_cap = v->capacity / 2;
    v = v->size < half_cap ? vector_client_resize(v, half_cap) : v;
    // pthread_mutex_unlock(&vetcor_mutex);
    return v;
}
