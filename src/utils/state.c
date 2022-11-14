#include "state.h"

#include <string.h>

#include "multithreading/mutex_wrappers.h"
#include "process/sig_handlers.h"
#include "utils/hash_map/hash_map.h"

struct state g_state = {
    // Server Environment
    .env = NULL,
    // Logging
    .log_level = LOG_LEVEL,
    .logging = false,
    .log_file_stream = NULL,
    // Multi-threading
    .max_num_threads = NUM_THREADS,
    .num_active_threads = 0,
    .thread_ids = NULL,
    .terminated_workers = NULL,
    .threads_mutex = PTHREAD_MUTEX_INITIALIZER,
    .default_lock_timeout = 3,
    // Job queue
    .job_queue = NULL,
    .job_queue_mutex = PTHREAD_MUTEX_INITIALIZER,
};

int setup_g_state(struct server_env *env)
{
    // Server Environment
    g_state.env = env;

    // Logging
    g_state.log_level = LOG_LEVEL;
    if (g_state.logging)
    {
        char *log_file_path = hash_map_get(env->config->global, "log_file");
        g_state.log_file_stream =
            log_file_path == NULL ? stdout : fopen(log_file_path, "w");
        if (g_state.log_file_stream == NULL)
        {
            log_error("%s: could not open log file \"%s\"\n", __func__,
                      log_file_path);
            return -1;
        }
    }
    else
        g_state.log_file_stream = NULL;

    // Multi-threading
    g_state.max_num_threads = NUM_THREADS;
    {
        int error;
        if ((error = init_mutex_wrapper(&g_state.threads_mutex)))
        {
            log_error("%s(threads mutex init): %s\n", __func__,
                      strerror(error));
            return -1;
        }
    }
    g_state.thread_ids = calloc(g_state.max_num_threads, sizeof(pthread_t));
    if (g_state.max_num_threads != 0 && g_state.thread_ids == NULL)
    {
        {
            int error;
            if ((error = pthread_mutex_destroy(&g_state.threads_mutex)))
                log_error("%s(thread ids oom: destroy mutex, ignore): %s\n",
                          __func__, strerror(error));
        }
        g_state.max_num_threads = 0;
        log_error("%s: Out of memory (thread_ids)\n", __func__);
        return -1;
    }
    g_state.terminated_workers = queue_init();
    if (g_state.terminated_workers == NULL)
    {
        log_error("%s(terminated workers queue init): failed to init queue\n",
                  __func__);
        pthread_mutex_destroy(&g_state.threads_mutex);
        free(g_state.thread_ids);
        return -1;
    }
    g_state.default_lock_timeout = 5;

    // Job queue
    g_state.job_queue = queue_init();
    if (g_state.job_queue == NULL)
    {
        log_error("%s(job queue init): failed to init queue\n", __func__);
        queue_destroy(g_state.terminated_workers);
        pthread_mutex_destroy(&g_state.threads_mutex);
        free(g_state.thread_ids);
        return -1;
    }

    if (init_mutex_wrapper(&g_state.job_queue_mutex) == -1)
    {
        pthread_mutex_destroy(&g_state.threads_mutex);
        free(g_state.thread_ids);
        log_error("%s: failed to initialize mutex for job queue\n", __func__);
        return -1;
    }

    return 0;
}

void set_g_state_logging(struct server_config *config)
{
    g_state.log_level = LOG_LEVEL;
    if (g_state.log_file_stream != NULL)
    {
        fclose(g_state.log_file_stream);
        g_state.log_file_stream = NULL;
    }
    char *log_value = hash_map_get(config->global, "log");
    g_state.logging = log_value == NULL ? true : strcmp(log_value, "true") == 0;
}
