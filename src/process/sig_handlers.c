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
    log_message(LOG_STDOUT | LOG_INFO, "\nGracefully killing self...\n");

    free_server_env(g_state.env, true, true);
    if (g_state.logging && g_state.log_file != NULL
        && g_state.log_file != stdout)
        fclose(g_state.log_file);

    exit(EXIT_SUCCESS);
}
