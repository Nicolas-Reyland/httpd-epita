#ifndef PARSER_REQUEST_H
#define PARSER_REQUEST_H

#include <stddef.h>

#include "utils/hash_map/hash_map.h"

struct request
{
    char *method;
    char *target;
    char *version;
    size_t body_size;
    char *body;
    struct hash_map *hash_map;
};

struct request *parser_request(char *raw_request, size_t size, int *err);
void free_request(struct request *req);

#endif /* !PARSER_REQUEST_H */
