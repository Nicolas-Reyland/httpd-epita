#ifndef VECTOR_STR_H
#define VECTOR_STR_H

#include <stddef.h>
#include <sys/types.h>

struct vector_str
{
    // The number of elements in the vector_str
    size_t size;
    // The maximum number of elements in the vector_str
    size_t capacity;
    // The elements themselves
    char **data;
};

/*
** Initialize the vector_str with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_str *vector_str_init(size_t n);

/*
** Release the memory used by the vector_str.
** Does nothing if `v` is `NULL`.
*/
void free_vector_str(struct vector_str *v);

/*
** Resize the vector_str to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector_str *vector_str_resize(struct vector_str *v, size_t n);

/*
** Append an element to the end of the vector_str. Expand the vector_str if
*needed.
** Returns `NULL` if an error occured.
*/
struct vector_str *vector_str_append(struct vector_str *v, char *s);

/*
** Returns the index of 's' in 'v' if it present, and -1 otherwise.
**/
ssize_t vector_str_find(struct vector_str *v, char *s);

/*
** Remove the element at the index `i`.
** Returns `NULL` if an error occured.
*/
struct vector_str *vector_str_remove(struct vector_str *v, size_t i);

#endif /* !VECTOR_STR_H */
