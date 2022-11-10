#ifndef ERROR_PARSING_H
#define ERROR_PARSING_H

enum error_parsing
{
    none = 0,
    METHOD_NOT_ALLOWED = 405,
    HOST_ERR = 400,
    VERSION_ERR = 505,
    REQUEST_ERR = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
};

#endif /* !ERROR_PARSING_H */
