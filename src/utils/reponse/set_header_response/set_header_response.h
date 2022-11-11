#ifndef SET_HEADER_RESPONSE_H
#define SET_HEADER_RESPONSE_H

#include "network/vhost.h"
#include "utils/reponse/reponse.h"
#include "utils/parsers/http/parser_request.h"
#include "utils/reponse/tools_response/tools_response.h"

void set_header_content_length(size_t content_len, struct response *resp);

void connexion_close_header(struct response *resp);

void set_header_server_name(struct response *resp, struct vhost *vhost);

struct response *set_error_response(char *path, char *ressource,
                                           struct response *resp, size_t *err);

void set_status_code_header(size_t *err, struct response *resp);

void set_date_gmt_header(struct response *resp);

#endif /* !SET_HEADER_RESPONSE_H */
