#include "my_getline.h"

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

ssize_t my_getdelim(char **lineptr, size_t *n, int delimiter, FILE *stream)
{
    ssize_t result;
    size_t cur_len = 0;

    if (lineptr == NULL || n == NULL || stream == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    if (*lineptr == NULL || *n == 0)
    {
        *n = 120;
        *lineptr = realloc(*lineptr, *n);
    }

    while (1)
    {
        int i = getc(stream);
        if (i == EOF)
        {
            result = -1;
            break;
        }

        /* Make enough space for len+1 (for final '\0') bytes.  */
        if (cur_len + 1 >= *n)
        {
            size_t needed_max = SIZE_MAX;
            size_t needed = 2 * *n + 1;

            if (needed_max < needed)
                needed = needed_max;
            if (cur_len + 1 >= needed)
            {
                result = -1;
                errno = EOVERFLOW;
                return result;
            }

            *lineptr = realloc(*lineptr, (*n = needed));
            // Disabling oom check, to save lines :)
            //            if (new_lineptr == NULL)
            //            {
            //                result = -1;
            //                return result;
            //            }
        }

        (*lineptr)[cur_len] = i;
        cur_len++;

        if (i == delimiter)
            break;
    }
    (*lineptr)[cur_len] = '\0';
    ssize_t signed_cur_len = cur_len;
    result = cur_len ? signed_cur_len : result;

    return result;
}
