#include "job.h"

#include <pthread.h>
#include <string.h>

#include "utils/logging.h"
#include "utils/state.h"

void add_job_to_queue(struct job job)
{
    {
        int error;
        if ((error = pthread_mutex_lock(&g_state.queue_mutex)))
        {
            log_error("%s(lock job queue): %s\n", __func__, strerror(error));
            return;
        }
    }

    if (job_queue_push(g_state.job_queue, job) == -1)
        log_error("%s: failed to push job to queue %d\n", __func__, job.type);

    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.queue_mutex)))
        {
            log_error("%s(unlock job queue): %s\n", __func__, strerror(error));
            return;
        }
    }
}
