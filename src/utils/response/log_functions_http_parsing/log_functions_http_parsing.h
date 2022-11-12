#ifndef LOG_FUNCTIONS_HTTP_PARSING_H
#define LOG_FUNCTIONS_HTTP_PARSING_H

#include <stddef.h>
#include <sys/types.h>

#include "network/vhost.h"
#include "utils/parsers/http/parser_request.h"
#include "utils/response/response.h"

void log_request(struct vhost *vhost, struct request *req, int *status_code,
                 ssize_t index);

void log_response(struct vhost *vhost, struct request *req, int *status_code,
                  ssize_t index);

#endif /* !LOG_FUNCTIONS_HTTP_PARSING_H */
