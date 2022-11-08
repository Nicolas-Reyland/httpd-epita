#ifndef PARSER_REQUEST_H
#define PARSER_REQUEST_H

#include "../../hash_map/hash_map.h"

struct request
{
    char *method;
    char *target;
    char *version;
    struct hash_map *hash_map;
};

struct request *parse_request(char *request);

#endif /* !PARSER_REQUEST_H */
