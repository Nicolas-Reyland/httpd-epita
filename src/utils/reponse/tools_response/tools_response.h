#ifndef TOOLS_RESPONSE_H
#define TOOLS_RESPONSE_H

#include "network/vhost.h"
#include "utils/reponse/reponse.h"
#include "utils/parsers/http/parser_request.h"

char *get_path_ressource(char *target, struct vhost *vhost);

char *put_ressource_resp(char *path, size_t *err, size_t *size);

void realloc_and_concat(struct response *resp, char *to_concat,
                        size_t to_concat_len, bool free_obj);

#endif /* !TOOLS_RESPONSE_H */
