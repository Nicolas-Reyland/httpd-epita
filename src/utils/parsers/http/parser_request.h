#ifndef PARSER_REQUEST_H
#define PARSER_REQUEST_H

#include "utils/hash_map/hash_map.h"
#include <stddef.h>

struct request
{
    char *method;
    char *target;
    char *version;
    size_t body_size;
    char *body;
    struct hash_map *hash_map;
};

struct request *parser_request(char *raw_request, size_t size);

#endif /* !PARSER_REQUEST_H */
