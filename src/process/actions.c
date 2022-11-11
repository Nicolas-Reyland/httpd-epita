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
    pid_t pid = (process_flags & DAEMONS_PID_MASK) >> 3;
    if (process_flags & DAEMONS_PROC_ALIVE)
        kill(pid, SIGINT);

    // daemonize self
    daemon(1, 1);
    start_all(config, pid_file);
}

_Noreturn void daemon_action_stop(char *pid_file, int process_flags)
{
    if (process_flags & DAEMONS_NO_PIDFILE)
    {
        log_message(LOG_STDOUT,
                    "Pid file %s does not exist, or is not readable\n",
                    pid_file);
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
    (void)pid_file;
    (void)process_flags;
    exit(2);
}

_Noreturn void daemon_action_restart(struct server_config *config,
                                     char *pid_file, int process_flags)
{
    (void)config;
    (void)pid_file;
    (void)process_flags;
    exit(2);
}
