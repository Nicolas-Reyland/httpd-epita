#include "signals.h"

#include <signal.h>
#include <stddef.h>
#include <string.h>

#include "handlers.h"
#include "utils/logging.h"

static void signal_handler(int signum);

int setup_signal_handlers(void)
{
    struct sigaction action;
    action.sa_handler = &signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
    sigaction(SIGUSR1, &action, NULL);
    sigaction(SIGUSR2, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGPIPE, &action, NULL);

    return 1;
}

void signal_handler(int signum)
{
    switch (signum)
    {
    case SIGINT:
    case SIGTERM:
        gracefull_shutdown();
        break;
    default:
        log_error("Unknown signal %s\n", strsignal(signum));
        break;
    }
}
