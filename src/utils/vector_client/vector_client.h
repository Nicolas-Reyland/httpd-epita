#ifndef VECTOR_CLIENT_H
#define VECTOR_CLIENT_H

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
struct client *vector_client_find(struct vector_client *v, int socket_fd);

/*
** Remove the element at the index `i`.
** Returns `NULL` if an error occured.
*/
struct vector_client *vector_client_remove(struct client *client);

#endif /* !VECTOR_CLIENT_H */
