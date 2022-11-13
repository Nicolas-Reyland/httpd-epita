#include "worker.h"

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "utils/logging.h"
#include "utils/state.h"

static ssize_t get_num_active_threads(bool keep_lock);

static void *worker_start_routine(void *ptr);

void start_worker(void)
{
    ssize_t num_active = get_num_active_threads(true);
    if (num_active == -1)
    {
        log_error("%s: could not retrieve number of active threads\n",
                  __func__);
        return;
    }
    size_t unum_active = num_active;

#ifdef _GNU_SOURCE
    // Try to join some threads first
    for (size_t i = 0; i < unum_active; ++i)
    {
        if (pthread_tryjoin_np(g_state.thread_ids[i], NULL) == 0)
        {
            --g_state.num_active_threads;
            log_debug("Joined thread %d\n", g_state.thread_ids[i]);
        }
    }
#endif /* _GNU_SOURCE */

    // Max active threads limit reached
    if (unum_active >= g_state.max_num_threads)
    {
        pthread_mutex_unlock(&g_state.num_active_threads_mutex);
        log_debug("%s: max number of threads reached : %zu\n", __func__,
                  g_state.max_num_threads);
        return;
    }

    // Start a new worker thread
    {
        int error;
        if ((error =
                 pthread_create(g_state.thread_ids + g_state.num_active_threads,
                                NULL, &worker_start_routine, NULL))
            == 0)
        {
            log_debug("Started worker thread %d\n",
                      g_state.thread_ids[g_state.num_active_threads]);
            ++g_state.num_active_threads;
        }
        else
            log_error("%s(create thread): %s\n", __func__, strerror(error));

        // Finally, release the mutex for the number of active threads
        if ((error = pthread_mutex_unlock(&g_state.num_active_threads_mutex)))
            log_error("%s(num_active_threads unlock): %s\n", __func__,
                      strerror(error));
    }
}

static ssize_t get_num_active_threads(bool keep_lock)
{
    int error;
    if ((error = pthread_mutex_lock(&g_state.num_active_threads_mutex)))
    {
        log_error("%s(num_active_threads lock): %s\n", __func__,
                  strerror(error));
        return -1;
    }

    ssize_t num_active = g_state.num_active_threads;

    if (!keep_lock
        && (error = pthread_mutex_unlock(&g_state.num_active_threads_mutex)))
    {
        log_error("%s(num_active_threads lock): %s\n", __func__,
                  strerror(error));
        // TODO: return -1 ??
    }

    return num_active;
}

static struct job get_next_job(void);

static void *worker_start_routine(void *ptr)
{
    log_debug("Currently in thread %d !\n", pthread_self());

    while (1)
    {
        struct job job = get_next_job();
        log_debug("[%d] job type is %d\n", pthread_self(), job.type);
        if (job.type == JOB_IDLE)
            break;
        execute_job(job);
    }

    // Decrement the num_active_threads
    ssize_t num_active = get_num_active_threads(true);
    if (num_active == -1)
    {
        log_error(
            "CRITICAL [%d] %s: could not lock num active threads\n"
            " - Definately losing one thread, until next join is called\n",
            pthread_self(), __func__);
        pthread_exit(NULL);
    }
    --g_state.num_active_threads;
    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.num_active_threads_mutex)))
            log_error("[%d] %s(num_active_threads unlock): %s\n",
                      pthread_self(), __func__, strerror(error));
    }
    log_debug("[%d] %s: finished work. exiting\n", pthread_self(), __func__);

    return ptr;
}

static struct job get_next_job(void)
{
    int error;
    if ((error = pthread_mutex_lock(&g_state.queue_mutex)))
    {
        log_error("[%d] %s(job queue lock): %s\n", pthread_self(), __func__,
                  strerror(error));
        // TODO: don't want infinite recursion, but kinda want to call
        // 'worker_start_routine' again ...
        pthread_exit(NULL);
    }

    struct job job = { .type = JOB_IDLE };
    job_queue_pop(g_state.job_queue, &job);

    if ((error = pthread_mutex_unlock(&g_state.queue_mutex)))
    {
        log_error("[%d] %s(job queue unlock): %s\n", pthread_self(), __func__,
                  strerror(error));
        pthread_exit(NULL);
    }

    return job;
}

void wait_for_workers(void)
{
    // TODO: this is very naive... should use IPC of some kind
    // TODO(2): join threads here ?
    while (get_num_active_threads(false) > 0)
        continue;
}
