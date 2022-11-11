#include "vector_str.h"

#include <stdlib.h>
#include <string.h>

#include "utils/logging.h"
#include "utils/mem.h"

/*
** Initialize the vector_str with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_str *vector_str_init(size_t n)
{
    char **data = calloc(n, sizeof(char *));
    if (data == NULL)
    {
        log_error("%s: Out of memory (data: %zu)\n", __func__, n);
        return NULL;
    }
    struct vector_str tmp = { 0, n, data };
    struct vector_str *v = malloc(sizeof(struct vector_str));
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
** Release the memory used by the vector_str.
** Does nothing if `v` is `NULL`.
*/
void free_vector_str(struct vector_str *v)
{
    if (v == NULL)
        return;
    for (size_t i = 0; i < v->size; ++i)
        free(v->data[i]);
    FREE_SET_NULL(v->data);
    FREE_SET_NULL(v);
}

/*
** Resize the vector_str to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_str *vector_str_resize(struct vector_str *v, size_t n)
{
    if (v == NULL)
        return NULL;

    v->data = realloc(v->data, (v->capacity = n) * sizeof(char *));
    if (v->data == NULL)
        return NULL;

    if (v->capacity < v->size)
        v->size = v->capacity;

    return v;
}

/*
** Append an element to the end of the vector_str. Expand the vector_str if
*needed.
** Returns `NULL` if an error occured.
*/
struct vector_str *vector_str_append(struct vector_str *v, char *s)
{
    if (v == NULL)
    {
        free(s);
        return NULL;
    }

    if (v->size == v->capacity)
    {
        v = vector_str_resize(v, 2 * v->capacity);
        if (v == NULL)
            return NULL;
    }

    v->data[v->size++] = s;
    return v;
}

/*
** Returns the index of 's' in 'v' if it present, and -1 otherwise.
**/
ssize_t vector_str_find(struct vector_str *v, char *s)
{
    if (v == NULL)
        return -1;

    for (size_t i = 0; i < v->size; ++i)
        if (strcmp(v->data[i], s) == 0)
            return i;

    return -1;
}

/*
** Remove the element at the index `i`.
** Replace it with the last element.
** Returns `NULL` if an error occured.
*/
struct vector_str *vector_str_remove(struct vector_str *v, size_t i)
{
    if (v == NULL || i >= v->size)
        return NULL;

    free(v->data[i]);
    v->data[i] = v->data[v->size - 1];
    --v->size;

    if (v->size < v->capacity / 2)
        return vector_str_resize(v, v->capacity / 2);

    return v;
}
