#ifndef TOOLS_RESPONSE_H
#define TOOLS_RESPONSE_H

#include "network/vhost.h"
#include "utils/parsers/http/parser_request.h"
#include "utils/response/response.h"

char *get_path_ressource(char *target, struct vhost *vhost);

char *put_ressource_resp(char *path, size_t *size, struct vhost *vhost,
                         size_t *err);

void realloc_and_concat(struct response *resp, char *to_concat,
                        size_t to_concat_len, bool free_obj);

#endif /* !TOOLS_RESPONSE_H */
