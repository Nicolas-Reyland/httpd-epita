#ifndef VECTOR_MUTEX_H
#define VECTOR_MUTEX_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#include "network/client.h"

struct vector_client
{
    // The number of elements in the vector_client
    size_t size;
    // The maximum number of elements in the vector_client
    size_t capacity;
    // The elements themselves
    struct client **data;
};

/*
** Initialize the vector_client with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_init(size_t n);

/*
** Release the memory used by the vector_client.
** Does nothing if `v` is `NULL`.
*/
void destroy_vector_client(struct vector_client *v);

/*
 * Returns validitity status of 'index' in 'v'
 */
bool vector_client_valid_index(struct vector_client *v, ssize_t index);

/*
** Append an element to the end of the vector_client. Expand the vector_client
*if needed.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_append(struct vector_client *v,
                                           struct client *client);

/*
 * Returns the index of the client (comparing socket file descriptors)
 * -1 if it is not found
 */
ssize_t vector_client_find(struct vector_client *v, int socket_fd);

/*
** Remove the element at the index `i`.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_remove(struct vector_client *v, size_t i);

#endif /* !VECTOR_MUTEX_H */
