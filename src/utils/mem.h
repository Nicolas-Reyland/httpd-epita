#ifndef MEM_H
#define MEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#define FCLOSE_SET_NULL(Ptr)                                                   \
    {                                                                          \
        free(Ptr);                                                             \
        (Ptr) = NULL;                                                          \
    }

#define FREE_SET_NULL(Ptr)                                                     \
    {                                                                          \
        free(Ptr);                                                             \
        Ptr = NULL;                                                            \
    }

void free_array(void **arr, size_t size, bool free_obj);

#endif /* !MEM_H */
