#ifndef RESPONSE_H
#define RESPONSE_H

#include "utils/parsers/http/parser_request.h"
#include "network/vhost.h"
#include <stddef.h>

struct response
{
    char *res;
    int err;
    size_t res_len;
};

struct response *create_response(int *err, char *vhost, char *target);

#endif /* !RESPONSE_H */
