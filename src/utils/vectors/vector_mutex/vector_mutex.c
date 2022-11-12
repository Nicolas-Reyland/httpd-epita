#include "vector_mutex.h"

#include <stdlib.h>
#include <string.h>

#include "utils/logging.h"
#include "utils/mem.h"

/*
** Initialize the vector_mutex with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_mutex *vector_mutex_init(size_t n)
{
    pthread_mutex_t *data = calloc(n, sizeof(pthread_mutex_t));
    if (data == NULL)
    {
        log_error("%s: Out of memory (data: %zu)\n", __func__, n);
        return NULL;
    }
    struct vector_mutex tmp = { 0, n, data };
    struct vector_mutex *v = malloc(sizeof(struct vector_mutex));
    if (v == NULL)
    {
        free(data);
        log_error("%s: Out of memory (struct)\n", __func__);
        return NULL;
    }
    *v = tmp;
    return v;
}

/*
** Release the memory used by the vector_mutex.
** Does nothing if `v` is `NULL`.
*/
void free_vector_mutex(struct vector_mutex *v)
{
    if (v == NULL)
        return;
    for (size_t i = 0; i < v->size; ++i)
        pthread_mutex_destroy(v->data + i);
    FREE_SET_NULL(v->data, v);
}

/*
** Resize the vector_mutex to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_mutex *vector_mutex_resize(struct vector_mutex *v, size_t n)
{
    if (v == NULL)
        return NULL;

    v->data = realloc(v->data, (v->capacity = n) * sizeof(pthread_mutex_t *));
    if (v->data == NULL)
        return NULL;

    if (v->capacity < v->size)
        v->size = v->capacity;

    return v;
}

/*
** Append an element to the end of the vector_mutex. Expand the vector_mutex if
*needed.
** Returns `NULL` if an error occured.
*/
struct vector_mutex *vector_mutex_append(struct vector_mutex *v,
                                         pthread_mutex_t mutex)
{
    if (v == NULL)
    {
        pthread_mutex_destroy(&mutex);
        return NULL;
    }

    if (v->size == v->capacity)
    {
        v = vector_mutex_resize(v, 2 * v->capacity);
        if (v == NULL)
            return NULL;
    }

    v->data[v->size++] = mutex;
    return v;
}

/*
** Remove the element at the index `i`.
** Replace it with the last element.
** Returns `NULL` if an error occured.
*/
struct vector_mutex *vector_mutex_remove(struct vector_mutex *v, size_t i)
{
    if (v == NULL || i >= v->size)
        return NULL;

    int error;
    error = pthread_mutex_destroy(v->data + i);
    v->data[i] = v->data[v->size - 1];
    --v->size;

    if (error)
    {
        log_error("%s: %s\n", __func__, strerror(error));
        return NULL;
    }

    if (v->size < v->capacity / 2)
        return vector_mutex_resize(v, v->capacity / 2);

    return v;
}
