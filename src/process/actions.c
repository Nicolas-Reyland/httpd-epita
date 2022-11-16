#include "actions.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "network/server.h"
#include "process/daemons.h"
#include "utils/logging.h"

_Noreturn void daemon_action_start(struct server_config *config, char *pid_file,
                                   int process_flags)
{
    log_debug("%s called with pid file %s\n", __func__, pid_file);

    pid_t pid = (process_flags & DAEMONS_PID_MASK) >> 3;
    if (process_flags & DAEMONS_PROC_ALIVE)
        kill(pid, SIGINT);

    daemon(1, 1);
    start_all(config, pid_file);
}

_Noreturn void daemon_action_stop(char *pid_file, int process_flags)
{
    log_debug("%s called with pid file %s\n", __func__, pid_file);

    if (process_flags & DAEMONS_NO_PIDFILE || process_flags & DAEMONS_PROC_DEAD)
    {
        log_warn("Pid file %s does not exist, or is not readable\n", pid_file);
        exit(EXIT_SUCCESS);
    }

    pid_t pid = (process_flags & DAEMONS_PID_MASK) >> 3;

    if (process_flags & DAEMONS_PROC_ALIVE)
    {
        kill(pid, SIGTERM);
    }
    if (unlink(pid_file) == -1)
    {
        log_error("%s: %s: %s", __func__, pid_file, strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

_Noreturn void daemon_action_reload(char *pid_file, int process_flags)
{
    log_debug("%s called with pid file %s\n", __func__, pid_file);

    pid_t pid = (process_flags & DAEMONS_PID_MASK) >> 3;
    log_debug("Calling reload signal on %d\n", pid);

    kill(pid, SIGUSR1);
    exit(EXIT_SUCCESS);
}

_Noreturn void daemon_action_restart(struct server_config *config,
                                     char *pid_file, int process_flags)
{
    log_debug("%s called with pid file %s\n", __func__, pid_file);

    if (process_flags & DAEMONS_PROC_ALIVE)
    {
        pid_t pid = (process_flags & DAEMONS_PID_MASK) >> 3;
        kill(pid, SIGTERM);
    }

    daemon(1, 1);
    start_all(config, pid_file);
}
