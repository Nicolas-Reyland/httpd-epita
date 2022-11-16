#include "my_getline.h"

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>

ssize_t my_getdelim(char **lineptr, size_t *n, int delimiter, FILE *stream)
{
    ssize_t result;
    size_t cur_len = 0;

    if (lineptr == NULL || n == NULL || stream == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    flockfile(stream);

    if (*lineptr == NULL || *n == 0)
    {
        *n = 120;
        char *new_lineptr = realloc(*lineptr, *n);
        if (new_lineptr == NULL)
        {
            result = -1;
            funlockfile(stream);
            return result;
        }
        *lineptr = new_lineptr;
    }

    while (1)
    {
        int i;

        i = getc(stream);
        if (i == EOF)
        {
            result = -1;
            break;
        }

        /* Make enough space for len+1 (for final NUL) bytes.  */
        if (cur_len + 1 >= *n)
        {
            size_t needed_max =
                SIZE_MAX;
            size_t needed = 2 * *n + 1;
            char *new_lineptr;

            if (needed_max < needed)
                needed = needed_max;
            if (cur_len + 1 >= needed)
            {
                result = -1;
                errno = EOVERFLOW;
                funlockfile(stream);
                return result;
            }

            new_lineptr = realloc(*lineptr, needed);
            if (new_lineptr == NULL)
            {
                result = -1;
                funlockfile(stream);
                return result;
            }

            *lineptr = new_lineptr;
            *n = needed;
        }

        (*lineptr)[cur_len] = i;
        cur_len++;

        if (i == delimiter)
            break;
    }
    (*lineptr)[cur_len] = '\0';
    ssize_t signed_cur_len = cur_len;
    result = cur_len ? signed_cur_len : result;

    funlockfile(stream);
    return result;
}
