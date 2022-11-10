#include "vector.h"

#include <stdio.h>
#include <stdlib.h>

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
** Display the vector contents on `stdout`.
** Displays `\n` if `v` is `NULL`.
*/
void vector_print(const struct vector *v)
{
    if (v == NULL || v->size == 0)
    {
        putchar('\n');
        return;
    }
    printf("%d", v->data[0]);
    for (size_t i = 1; i < v->size; i++)
        printf(",%d", v->data[i]);
    putchar('\n');
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
** Insert `n` at the index `i`. Expand the vector if needed.
** Returns `NULL` if an error occured.
*/
struct vector *vector_insert(struct vector *v, size_t i, int elt)
{
    if (v == NULL || i > v->size)
        return NULL;

    if (v->size == v->capacity)
        v = vector_resize(v, 2 * v->capacity);

    for (size_t j = v->size; j > i; --j)
        v->data[j] = v->data[j - 1];
    ++v->size;

    v->data[i] = elt;
    return v;
}

/*
** Remove the element at the index `i`.
** Returns `NULL` if an error occured.
*/
struct vector *vector_remove(struct vector *v, size_t i)
{
    if (v == NULL || i >= v->size)
        return NULL;

    --v->size;
    for (size_t j = i; j < v->size; ++j)
        v->data[j] = v->data[j + 1];

    if (v->size < v->capacity / 2)
        return vector_resize(v, v->capacity / 2);

    return v;
}
