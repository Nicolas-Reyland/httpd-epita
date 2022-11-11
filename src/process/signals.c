#include "signals.h"

#include <signal.h>
#include <stddef.h>

#include "sig_handlers.h"
#include "utils/logging.h"

int setup_signal_handlers(void)
{
    struct sigaction action;
    action.sa_handler = &signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) == -1
        || sigaction(SIGUSR1, &action, NULL) == -1
        || sigaction(SIGUSR2, &action, NULL) == -1
        || sigaction(SIGTERM, &action, NULL) == -1
        || sigaction(SIGPIPE, &action, NULL) == -1)
    {
        log_error("%s: sigaction failed\n", __func__);
        return -1;
    }

    return 0;
}
