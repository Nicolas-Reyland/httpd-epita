#ifndef LOG_FUNCTIONS_HTTP_PARSING_H
#define LOG_FUNCTIONS_HTTP_PARSING_H

#include <stddef.h>
#include <sys/types.h>

#include "network/vhost.h"
#include "response/response.h"
#include "utils/parsers/http/parser_request.h"

void log_request(struct client *client, struct request *req, int *status_code);

void log_response(struct client *client, struct request *req, int *status_code);

#endif /* !LOG_FUNCTIONS_HTTP_PARSING_H */
