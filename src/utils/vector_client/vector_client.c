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
 * Returns validitity status of 'index' in 'v'
 */
bool vector_valid_index(struct vector_client *v, ssize_t index);

/*
 * Returns the index of the client (comparing socket file descriptors)
 * -1 if it is not found
 */
ssize_t vector_client_find(struct vector_client *v, int socket_fd)
{
    for (ssize_t i = 0; i < v->size; ++i)
    {
        //
    }

    return -1;
}
/*
** Resize the vector_client to `n` capacity.
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
    return v == NULL ? false : index >= 0 && index < v->size;
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

    v->data[v->size++] = client;
    return v;
}

/*
** Remove the element at the index `i`.
** Replace it with the last element.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_remove(struct vector_client *v, size_t i)
{
    if (v == NULL || i >= v->size)
        return NULL;

    destroy_client(v->data[i], true);
    v->data[i] = v->data[v->size - 1];
    --v->size;

    if (v->size < v->capacity / 2)
        return vector_client_resize(v, v->capacity / 2);

    return v;
}
