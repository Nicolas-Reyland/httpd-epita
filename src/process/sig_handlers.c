#include "sig_handlers.h"

#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network/server_env.h"
#include "network/vhost.h"
#include "utils/logging.h"
#include "utils/reload_config.h"
#include "utils/state.h"

void signal_handler(int signum)
{
    switch (signum)
    {
    case SIGINT:
    case SIGTERM:
        graceful_shutdown();
        break;
    case SIGPIPE:
        log_warn("Caught SIGPIPE :(\n");
        break;
    case SIGUSR1:
        reload_config();
        break;
#ifdef CATCH_SIGUSR2
    case SIGUSR2:
        if (g_state.log_level == LOG_EPITA)
        {
            g_state.log_level = LOG_DEBUG;
            log_debug("setting log level to LOG_DEBUG\n");
        }
        else
        {
            log_debug("setting log level to LOG_EPITA\n");
            g_state.log_level = LOG_EPITA;
        }
        break;
#endif /* CATCH_SIGUSR2 */
    default:
        log_error("caught unknown signal: %s\n", strsignal(signum));
        break;
    }
}

_Noreturn void graceful_shutdown(void)
{
    log_info("Gracefully killing self...\n\n");

    free_server_env(g_state.env, true, true);

    // Join all the active threads
    pthread_mutex_lock(&g_state.num_active_threads_mutex);
    for (size_t i = 0; i < g_state.num_active_threads; ++i)
        pthread_join(g_state.thread_ids[i], NULL);
    pthread_mutex_unlock(&g_state.num_active_threads_mutex);
    pthread_mutex_destroy(&g_state.num_active_threads_mutex);
    // Destroy thread ids buffer
    free(g_state.thread_ids);

    // Close logging stream if needed
    if (g_state.log_file_stream != NULL && g_state.log_file_stream != stdout)
        fclose(g_state.log_file_stream);

    // Destroy job queue in a thread-safe way
    pthread_mutex_lock(&g_state.queue_mutex);
    job_queue_destroy(g_state.job_queue);
    g_state.job_queue = NULL;
    pthread_mutex_unlock(&g_state.queue_mutex);
    pthread_mutex_destroy(&g_state.queue_mutex);

    exit(EXIT_SUCCESS);
}
