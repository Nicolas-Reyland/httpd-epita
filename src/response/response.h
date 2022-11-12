#ifndef RESPONSE_H
#define RESPONSE_H

#include <stddef.h>
#include <sys/types.h>

#include "network/vhost.h"
#include "utils/parsers/http/parser_request.h"

struct response
{
    char *res;
    size_t res_len;
    int fd;
    size_t file_len;
    int err;
};

struct response *parsing_http(char *request_raw, size_t size, struct client *client);
void free_response(struct response *resp);

#endif /* !RESPONSE_H */
