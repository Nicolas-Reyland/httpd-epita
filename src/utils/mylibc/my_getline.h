#ifndef MY_GETLINE_H
#define MY_GETLINE_H

#include <stdio.h>
#include <unistd.h>

ssize_t my_getdelim(char **lineptr, size_t *n, int delimiter, FILE *fp);

inline ssize_t my_getline(char **lineptr, size_t *n, FILE *stream)
{
    return my_getdelim(lineptr, n, '\n', stream);
}

#endif /* !MY_GETLINE_H */
