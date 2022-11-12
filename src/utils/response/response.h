#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>
#include <sys/types.h>

#include "network/vhost.h"
#include "utils/parsers/http/parser_request.h"

struct response
{
    char *res;
    int err;
    size_t res_len;
};

struct response *parsing_http(char *request_raw, size_t size,
                              struct vhost *vhost, ssize_t index);
void free_response(struct response *resp);

#endif /* !RESPONSE_H */
