#include "state.h"

#include <string.h>

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
    .num_threads = NUM_THREADS,
    .thread_ids = NULL,
    .job_queue = NULL,
    .queue_mutex = PTHREAD_MUTEX_INITIALIZER,
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
            graceful_shutdown();
        }
    }
    else
        g_state.log_file_stream = NULL;

    // Multi-threading
    g_state.num_threads = NUM_THREADS;
    g_state.thread_ids = calloc(g_state.num_threads, sizeof(pthread_t));
    if (g_state.num_threads != 0 && g_state.thread_ids == NULL)
    {
        log_error("%s: Out of memory (thread_ids)\n", __func__);
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
