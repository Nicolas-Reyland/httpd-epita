#include "vector.h"

#include <stdlib.h>

#include "utils/mem.h"

/*
** Initialize the vector with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector *vector_init(size_t n)
{
    int *data = malloc(n * sizeof(int));
    if (data == NULL)
        return NULL;
    struct vector tmp = { 0, n, data };
    struct vector *v = malloc(sizeof(struct vector));
    *v = tmp;
    return v;
}

/*
** Release the memory used by the vector.
** Does nothing if `v` is `NULL`.
*/
void free_vector(struct vector *v)
{
    if (v == NULL)
        return;
    FREE_SET_NULL(v->data);
    FREE_SET_NULL(v);
}

/*
** Resize the vector to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector *vector_resize(struct vector *v, size_t n)
{
    if (v == NULL)
        return NULL;

    v->data = realloc(v->data, (v->capacity = n) * sizeof(int));
    if (v->data == NULL)
        return NULL;

    if (v->capacity < v->size)
        v->size = v->capacity;

    return v;
}

/*
** Append an element to the end of the vector. Expand the vector if needed.
** Returns `NULL` if an error occured.
*/
struct vector *vector_append(struct vector *v, int elt)
{
    if (v == NULL)
        return NULL;

    if (v->size == v->capacity)
    {
        v = vector_resize(v, 2 * v->capacity);
        if (v == NULL)
            return NULL;
    }

    v->data[v->size++] = elt;
    return v;
}

/*
** Remove all the elements of the vector, and resize it to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector *vector_reset(struct vector *v, size_t n)
{
    v->data = realloc(v->data, (v->capacity = n) * sizeof(int));
    if (v->data == NULL)
        return NULL;
    v->size = 0;
    return v;
}

/*
** Returns the index of 'elt' in 'v' if it present, and -1 otherwise.
**/
ssize_t vector_find(struct vector *v, int elt)
{
    if (v == NULL)
        return -1;

    for (size_t i = 0; i < v->size; ++i)
        if (v->data[i] == elt)
            return i;

    return -1;
}

/*
** Remove the element at the index `i`.
** Replace it with the last element.
** Returns `NULL` if an error occured.
*/
struct vector *vector_remove(struct vector *v, size_t i)
{
    if (v == NULL || i >= v->size)
        return NULL;

    v->data[i] = v->data[v->size - 1];
    --v->size;

    if (v->size < v->capacity / 2)
        return vector_resize(v, v->capacity / 2);

    return v;
}