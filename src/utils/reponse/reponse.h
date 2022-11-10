#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>

struct response
{
    char *res;
    int err;
    size_t res_len;
};

#endif /* !RESPONSE_H */
