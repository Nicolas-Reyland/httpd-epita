#ifndef SET_HEADER_RESPONSE_H
#define SET_HEADER_RESPONSE_H

#include "network/vhost.h"
#include "utils/parsers/http/parser_request.h"
#include "response/response.h"
#include "response/tools_response/tools_response.h"

void set_header_content_length(size_t content_len, struct response *resp);

void connexion_close_header(struct response *resp);

void set_header_server_name(struct response *resp, struct vhost *vhost);

struct response *set_error_response(struct vhost *vhost, struct response *resp,
                                    int( *err));

void set_status_code_header(int( *err), struct response *resp);

void set_date_gmt_header(struct response *resp);

#endif /* !SET_HEADER_RESPONSE_H */
