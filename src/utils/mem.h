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

#define FREE_SET_NULL(...)                                                     \
    {                                                                          \
        void *_Free_Set_Null_Vargs[] = { __VA_ARGS__ };                        \
        size_t _Free_Set_Null_Num_Vargs =                                      \
            sizeof(_Free_Set_Null_Vargs) / sizeof(_Free_Set_Null_Vargs[0]);    \
        for (size_t _Free_Set_Null_Vargs_Index = 0;                            \
             _Free_Set_Null_Vargs_Index < _Free_Set_Null_Num_Vargs;            \
             ++_Free_Set_Null_Vargs_Index)                                     \
        {                                                                      \
            void *_Free_Set_Null_Arg =                                         \
                _Free_Set_Null_Vargs[_Free_Set_Null_Vargs_Index];              \
            free(_Free_Set_Null_Arg);                                          \
            _Free_Set_Null_Arg = NULL;                                         \
        }                                                                      \
    }

#define CLOSE_ALL(...)                                                         \
    {                                                                          \
        int _Close_Set_Null_Vargs[] = { __VA_ARGS__ };                         \
        size_t _Close_Set_Null_Num_Vargs =                                     \
            sizeof(_Close_Set_Null_Vargs) / sizeof(_Close_Set_Null_Vargs[0]);  \
        for (size_t _Close_Set_Null_Vargs_Index = 0;                           \
             _Close_Set_Null_Vargs_Index < _Close_Set_Null_Num_Vargs;          \
             ++_Close_Set_Null_Vargs_Index)                                    \
        {                                                                      \
            int _Close_Set_Null_Arg =                                          \
                _Close_Set_Null_Vargs[_Close_Set_Null_Vargs_Index];            \
            if (_Close_Set_Null_Arg == -1)                                     \
            {                                                                  \
                continue;                                                      \
            }                                                                  \
            close(_Close_Set_Null_Arg);                                        \
            _Close_Set_Null_Arg = -1;                                          \
        }                                                                      \
    }

void free_array(void **arr, size_t size, bool free_obj);

#endif /* !MEM_H */
