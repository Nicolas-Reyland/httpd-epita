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
        log_error("[%d] %s: could not retrieve number of active threads\n",
                  pthread_self(), __func__);
        return;
    }
    size_t unum_active = num_active;

#ifdef _GNU_SOURCE
    // Try to join some threads first
    for (size_t i = 0; i < unum_active; ++i)
    {
        if (pthread_tryjoin_np(g_state.thread_ids[i], NULL) == 0)
        {
            log_debug("Joined thread %d\n", g_state.thread_ids[i]);
            g_state.thread_ids[i] = 0;
        }
    }
#endif /* _GNU_SOURCE */

    // Max active threads limit reached
    if (unum_active >= g_state.max_num_threads)
    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.threads_mutex)))
            log_error("%s(max num active threads reached, ignore): %s\n",
                      pthread_self(), __func__);
        log_debug("%s: max number of threads reached : %zu\n", __func__,
                  g_state.max_num_threads);
        return;
    }

    size_t thread_id_index;
    for (thread_id_index = 0; thread_id_index < g_state.max_num_threads;
         ++thread_id_index)
        if (g_state.thread_ids[thread_id_index] == 0)
            break;

    int valid_thread_index = thread_id_index != g_state.max_num_threads;
    void *thread_id_buff = NULL;

    if (!valid_thread_index)
        log_error("%s: no valid thread id found (zero). not starting worker\n",
                  __func__);
    else if ((thread_id_buff = malloc(sizeof(size_t))) == NULL)
        log_error("%s(alloc thread id buff): Out of memory\n", __func__);
    // Start a new worker thread
    else
    {
        int error;
        if ((error = pthread_create(g_state.thread_ids + thread_id_index, NULL,
                                    &worker_start_routine, thread_id_buff)))
            log_error("%s(create thread): %s\n", __func__, strerror(error));
        else
        {
            log_debug("Started worker thread [%zu : %d]\n", thread_id_index,
                      g_state.thread_ids[thread_id_index]);
            ++g_state.num_active_threads;
        }
    }

    // Finally, release the mutex for the number of active threads
    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.threads_mutex)))
            log_error("%s(num_active_threads unlock): %s\n", __func__,
                      strerror(error));
    }
}

static ssize_t get_num_active_threads(bool keep_lock)
{
    int error;
    if ((error = pthread_mutex_lock(&g_state.threads_mutex)))
    {
        log_error("%s(threads lock): %s\n", __func__, strerror(error));
        return -1;
    }

    ssize_t num_active = g_state.num_active_threads;
    if (num_active == -1)
        log_warn(
            "[%d] %s: num active threads is %zd. this might cause errors\n",
            pthread_self(), __func__, num_active);

    if (!keep_lock && (error = pthread_mutex_unlock(&g_state.threads_mutex)))
    {
        log_error("%s(threads unlock): %s\n", __func__, strerror(error));
        // TODO: return -1 ??
    }

    return num_active;
}

static struct job get_next_job(void);

static void *worker_start_routine(void *ptr)
{
    free(ptr);

    while (1)
    {
        struct job job = get_next_job();
        log_debug("[%d] job type is %d\n", pthread_self(), job.type);
        if (job.type == JOB_IDLE)
            break;
        execute_job(job);
    }

    log_info("[%d] %s: no jobs left. exiting.\n", pthread_self(), __func__);

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
    // g_state.thread_ids[thread_id_index] = 0;
    --g_state.num_active_threads;
    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.threads_mutex)))
            log_error("[%d] %s(num_active_threads unlock): %s\n",
                      pthread_self(), __func__, strerror(error));
        log_debug("[%d] %s: finished work. exiting\n", pthread_self(),
                  __func__);
    }

    return NULL;
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
