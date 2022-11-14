#ifndef JOB_H
#define JOB_H

#include <stddef.h>
#include <sys/types.h>

#include "multithreading/job/job_type.h"
#include "network/client.h"
#include "response/response.h"

struct job
{
    enum job_type type;
    int socket_fd;
    ssize_t index;
    // for debugging purposes only
    size_t uid;
};

void add_job_to_queue(struct job *job);

void execute_job(struct job *job);

#endif /* !JOB_H */
