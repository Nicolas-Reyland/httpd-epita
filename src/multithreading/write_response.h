#ifndef WRITE_RESPONSE_H
#define WRITE_RESPONSE_H

#include <sys/types.h>

#include "network/client.h"
#include "utils/response/response.h"

int write_response(struct client *client, struct response *resp);

#endif /* !WRITE_RESPONSE_H */
