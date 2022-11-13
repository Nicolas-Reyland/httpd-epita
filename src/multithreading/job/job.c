#include "job.h"

#include <pthread.h>
#include <string.h>

#include "network/server_env.h"
#include "utils/logging.h"
#include "utils/state.h"

void add_job_to_queue(struct job job)
{
    log_debug("Adding job of type %d to queue\n", job.type);

    {
        int error;
        if ((error = pthread_mutex_lock(&g_state.queue_mutex)))
        {
            log_error("%s(lock job queue): %s\n", __func__, strerror(error));
            return;
        }
    }

    if (job_queue_push(g_state.job_queue, job) == -1)
        log_error("%s: failed to push job of type %d to queue\n", __func__,
                  job.type);

    log_debug("%s: there are %zu jobs waiting in queue\n", __func__,
              g_state.job_queue->size);

    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.queue_mutex)))
        {
            log_error("%s(unlock job queue): %s\n", __func__, strerror(error));
            return;
        }
    }
}

static void execute_accept_job(struct job job);

static void execute_process_job(struct job job);

static void execute_close_job(struct job job);

void execute_job(struct job job)
{
    switch (job.type)
    {
    case JOB_ACCEPT:
        execute_accept_job(job);
        break;
    case JOB_PROCESS:
        execute_process_job(job);
        break;
    case JOB_CLOSE:
        execute_close_job(job);
        break;
    case JOB_IDLE:
        log_warn("%s: idle job passed down to execution\n", __func__);
        break;
    default:
        log_warn("%s: unkown job type %d\n", __func__, job.type);
        break;
    }
}

void execute_accept_job(struct job job)
{
    (void)job;
    log_debug("Executing access job\n");
}

void execute_process_job(struct job job)
{
    (void)job;
    log_debug("Executing process job\n");
}

void execute_close_job(struct job job)
{
    (void)job;
    log_debug("Executing close job\n");
}
