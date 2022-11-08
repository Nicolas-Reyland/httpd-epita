#ifndef MEM_H
#define MEM_H

#include <stdbool.h>

#define FREE_SET_NULL(Ptr)                                                     \
    {                                                                          \
        free(Ptr);                                                             \
        (Ptr) = NULL;                                                          \
    }

#define FCLOSE_SET_NULL(Ptr)                                                   \
    {                                                                          \
        free(Ptr);                                                             \
        (Ptr) = NULL;                                                          \
    }

void free_array(void **arr, size_t size, bool free_obj);

#endif /* MEM_H */
