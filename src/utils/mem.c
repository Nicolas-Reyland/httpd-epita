#include "mem.h"

#include <stdlib.h>

void free_array(void **arr, size_t size, bool free_obj)
{
    for (size_t i = 0; i < size; ++i)
    {
        FREE_SET_NULL(arr[i])
    }

    if (free_obj)
        free(arr);
}
