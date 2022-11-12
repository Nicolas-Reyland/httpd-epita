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
    default:
        log_error("Unknown signal %s\n", strsignal(signum));
        break;
    }
}

_Noreturn void graceful_shutdown(void)
{
    log_warn("\nGracefully killing self...\n");

    free_server_env(g_state.env, true, true);

    /*
     * TODO: uncomment this when worker threads are implemented
    for (size_t i = 0; i < g_state.num_threads; ++i)
        pthread_join(g_sate.thread_ids[i], NULL);
    */
    free(g_state.thread_ids);

    if (g_state.log_file_stream != NULL && g_state.log_file_stream != stdout)
        fclose(g_state.log_file_stream);

    pthread_mutex_lock(&g_state.queue_mutex);
    job_queue_destroy(g_state.job_queue);
    pthread_mutex_unlock(&g_state.queue_mutex);
    pthread_mutex_destroy(&g_state.queue_mutex);

    exit(EXIT_SUCCESS);
}
