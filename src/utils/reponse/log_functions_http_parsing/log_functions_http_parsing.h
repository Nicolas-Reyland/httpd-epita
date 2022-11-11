#ifndef LOG_FUNCTIONS_HTTP_PARSING_H
#define LOG_FUNCTIONS_HTTP_PARSING_H

#include "network/vhost.h"
#include "utils/reponse/reponse.h"
#include "utils/parsers/http/parser_request.h"

void log_request(struct vhost *vhost, struct request *req, size_t *status_code);

void log_response(struct vhost *vhost, struct request *req, size_t *status_code);

#endif /* !LOG_FUNCTIONS_HTTP_PARSING_H */
