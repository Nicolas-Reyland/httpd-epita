#ifndef THREAD_SAFE_WRITE_H
#define THREAD_SAFE_WRITE_H

#include <sys/types.h>

#include "network/vhost.h"
#include "response/response.h"

int thread_safe_write(struct vhost *vhost, ssize_t index,
                      struct response *resp);

#endif /* !THREAD_SAFE_WRITE_H */
