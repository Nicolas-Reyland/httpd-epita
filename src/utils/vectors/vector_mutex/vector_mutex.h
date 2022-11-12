#ifndef VECTOR_MUTEX_H
#define VECTOR_MUTEX_H

#include <pthread.h>
#include <stddef.h>
#include <sys/types.h>

struct vector_mutex
{
    // The number of elements in the vector_mutex
    size_t size;
    // The maximum number of elements in the vector_mutex
    size_t capacity;
    // The elements themselves
    pthread_mutex_t *data;
};

/*
** Initialize the vector_mutex with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_mutex *vector_mutex_init(size_t n);

/*
** Release the memory used by the vector_mutex.
** Does nothing if `v` is `NULL`.
*/
void free_vector_mutex(struct vector_mutex *v);

/*
** Resize the vector_mutex to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_mutex *vector_mutex_resize(struct vector_mutex *v, size_t n);

/*
** Append an element to the end of the vector_mutex. Expand the vector_mutex if
*needed.
** Returns `NULL` if an error occured.
*/
struct vector_mutex *vector_mutex_append(struct vector_mutex *v,
                                         pthread_mutex_t mutex);

/*
** Remove the element at the index `i`.
** Returns `NULL` if an error occured.
*/
struct vector_mutex *vector_mutex_remove(struct vector_mutex *v, size_t i);

#endif /* !VECTOR_MUTEX_H */
