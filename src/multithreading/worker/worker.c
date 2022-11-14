#include "worker.h"

#include <stdbool.h>
#include <string.h>
#include <sys/types.h>

#include "multithreading/job/job.h"
#include "multithreading/mutex_wrappers.h"
#include "utils/logging.h"
#include "utils/state.h"

static ssize_t get_num_active_threads(bool keep_lock);

static void *worker_start_routine(void *ptr);

void start_worker(int socket_fd)
{
    ssize_t num_active = get_num_active_threads(true);
    if (num_active == -1)
    {
        log_error("[%u] %s: could not retrieve number of active threads\n",
                  pthread_self(), __func__);
        return;
    }
    size_t unum_active = num_active;

    // Max active threads limit reached
    if (unum_active >= g_state.max_num_threads)
    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.threads_mutex)))
            log_error("%s(early unlock threads mutex, ignore): %s\n",
                      pthread_self(), __func__);
        log_info("%s: max number of threads reached : %zu\n", __func__,
                 g_state.max_num_threads);
        return;
    }

    size_t thread_index = socket_fd % g_state.max_num_threads;
    if (g_state.thread_ids[thread_index])
    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.threads_mutex)))
            log_error("%s(early unlock threads mutex, ignore): %s\n",
                      pthread_self(), __func__);
        log_info("Thread is already running\n");
        return;
    }

    int valid_thread_index = thread_index != g_state.max_num_threads;
    size_t *thread_index_buff = malloc(sizeof(size_t));

    // Validate data
    if (!valid_thread_index)
        log_error("%s: no valid thread id found (zero). not starting worker\n",
                  __func__);
    else if (thread_index_buff == NULL)
        log_error("%s(alloc thread id buff): Out of memory\n", __func__);

    // Start a new worker thread
    else
    {
        *thread_index_buff = thread_index;
        int error;
        if ((error = pthread_create(g_state.thread_ids + thread_index, NULL,
                                    &worker_start_routine, thread_index_buff)))
            log_error("%s(create thread): %s\n", __func__, strerror(error));
        else
        {
            log_debug("Started worker thread [%zu : %d]\n", thread_index,
                      g_state.thread_ids[thread_index]);
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
    if ((error = lock_mutex_wrapper(&g_state.threads_mutex)))
    {
        log_error("%s(threads lock): %s\n", __func__, strerror(error));
        return -1;
    }

    ssize_t num_active = g_state.num_active_threads;
    if (num_active < 0)
        log_warn(
            "[%u] %s: num active threads is %zd. this might cause errors\n",
            pthread_self(), __func__, num_active);

    if (!keep_lock && (error = pthread_mutex_unlock(&g_state.threads_mutex)))
    {
        log_error("%s(threads unlock): %s\n", __func__, strerror(error));
        // TODO: return -1 ??
    }

    return num_active;
}

static struct job *get_next_job(size_t thread_index);

static void *worker_start_routine(void *ptr)
{
    size_t *thread_index_ptr = ptr;
    size_t thread_index = *thread_index_ptr;
    free(ptr);

    // Execute jobs while there are some left
    while (1)
    {
        struct job *job = get_next_job(thread_index);
        if (job == NULL || job->type == JOB_IDLE)
            break;
        log_debug("[%u] job type is %d\n", pthread_self(), job->type);
        execute_job(job);
    }

    log_info("[%u] %s: no jobs left. exiting.\n", pthread_self(), __func__);

    // Lock thread-managing structures
    ssize_t num_active = get_num_active_threads(true);
    if (num_active == -1)
    {
        log_error(
            "CRITICAL [%u] %s: could not lock num active threads\n"
            " - Definately losing one thread, until next join is called\n",
            pthread_self(), __func__);
        pthread_exit(NULL);
    }

    // Clean up thread
    log_debug("%s: cleaning up thread [%zu: %d]\n", __func__, thread_index,
              pthread_self());
    // Add self thread id for later joining in the main thread
    pthread_t *self_thread_id = malloc(sizeof(pthread_t));
    if (self_thread_id == NULL)
        log_error("%s(self thread id): Out of memory\n");
    else
    {
        *self_thread_id = pthread_self();
        queue_push(g_state.terminated_workers, self_thread_id);
    }
    // Make fere space for later new threads
    g_state.thread_ids[thread_index] = 0;
    --g_state.num_active_threads;

    // Unlock thread-managing structures
    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.threads_mutex)))
            log_error("[%u] %s(num_active_threads unlock): %s\n",
                      pthread_self(), __func__, strerror(error));
    }

    log_debug("[%u] %s: finished work. exiting\n", pthread_self(), __func__);

    return NULL;
}

static struct job *get_next_job(size_t thread_index)
{
    // TODO: check thread index

    int error;
    if ((error = lock_mutex_wrapper(&g_state.job_queues_mutexes[thread_index])))
    {
        log_error("[%u] %s(job queue lock): %s\n", pthread_self(), __func__,
                  strerror(error));
        // This is really bad ...
        return NULL;
    }

    struct job *job = queue_pop(g_state.job_queues[thread_index]);

    if ((error =
             pthread_mutex_unlock(&g_state.job_queues_mutexes[thread_index])))
    {
        log_error("[%u] %s(job queue unlock): %s\n", pthread_self(), __func__,
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
