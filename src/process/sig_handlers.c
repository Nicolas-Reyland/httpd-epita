#include "sig_handlers.h"

#include <errno.h>
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
    int error;
    if ((error = pthread_mutex_lock(&g_state.threads_mutex)))
        log_error("%s(lock num_active_threads, continue): %s\n", __func__,
                  strerror(error));
    else
    {
        for (size_t i = 0; i < g_state.num_active_threads; ++i)
        {
            log_info("%s: thread %d\n", __func__, g_state.thread_ids[i]);
            if ((error = pthread_join(g_state.thread_ids[i], NULL)))
                log_error("%s(join thread %d, continue): %s\n", __func__,
                          pthread_self(), strerror(error));
        }
        if ((error = pthread_mutex_unlock(&g_state.threads_mutex)))
            log_error("%s(unlock num_active_threads, ignore): %s\n", __func__,
                      strerror(error));
        if ((error = pthread_mutex_destroy(&g_state.threads_mutex)))
            log_error("%s(destroy num_active_threads, ignore): %s\n", __func__,
                      strerror(error));
    }
    // Destroy thread ids buffer
    free(g_state.thread_ids);

    // Close logging stream if needed
    if (g_state.log_file_stream != NULL && g_state.log_file_stream != stdout
        && fclose(g_state.log_file_stream) != 0)
        log_error("%s(fclose log_file_stream, ignore): %s\n", __func__,
                  strerror(errno));

    // Destroy job queue in a thread-safe way
    if ((error = pthread_mutex_lock(&g_state.queue_mutex)))
        log_error("%s(lock queue, ignore): %s\n", __func__, strerror(error));
    job_queue_destroy(g_state.job_queue);
    g_state.job_queue = NULL;
    if ((error = pthread_mutex_unlock(&g_state.queue_mutex)))
        log_error("%s(lock queue, ignore): %s\n", __func__, strerror(error));
    if ((error = pthread_mutex_destroy(&g_state.queue_mutex)))
        log_error("%s(lock queue, ignore): %s\n", __func__, strerror(error));

    exit(EXIT_SUCCESS);
}
