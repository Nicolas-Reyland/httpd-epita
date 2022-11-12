#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <sys/types.h>

struct vector
{
    // The number of elements in the vector
    size_t size;
    // The maximum number of elements in the vector
    size_t capacity;
    // The elements themselves
    int *data;
};

/*
** Initialize the vector with `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector *vector_init(size_t n);

/*
** Release the memory used by the vector.
** Does nothing if `v` is `NULL`.
*/
void free_vector(struct vector *v);

/*
** Resize the vector to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector *vector_resize(struct vector *v, size_t n);

/*
** Append an element to the end of the vector. Expand the vector if needed.
** Returns `NULL` if an error occured.
*/
struct vector *vector_append(struct vector *v, int elt);

/*
** Remove all the elements of the vector, and resize it to `n` capacity.
** Returns `NULL` if an error occured.
*/
struct vector *vector_reset(struct vector *v, size_t n);

/*
** Returns the index of 'elt' in 'v' if it present, and -1 otherwise.
**/
ssize_t vector_find(struct vector *v, int elt);

/*
** Remove the element at the index `i`.
** Returns `NULL` if an error occured.
*/
struct vector *vector_remove(struct vector *v, size_t i);

#endif /* !VECTOR_H */