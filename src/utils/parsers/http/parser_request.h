#ifndef PARSER_REQUEST_H
#define PARSER_REQUEST_H

struct request
{
    char *method;
    char *target;
    char *version;
};

struct request *parse_request(char *request);

#endif /* !PARSER_REQUEST_H */
