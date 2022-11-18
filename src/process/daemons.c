#include "daemons.h"

#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "process/actions.h"
#include "utils/arg_flags.h"
#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"

#define PID_STR_BUFF_SIZE 1024

static int retrieve_process_flags(char *pid_file);

_Noreturn void handle_daemon(struct server_config *config, int flags)
{
    char *pid_file = hash_map_get(config->global, "pid_file");
    if (pid_file == NULL)
    {
        log_error("%s: no pid file in server config\n", __func__);
        exit(EXIT_FAILURE);
    }
    int process_flags = retrieve_process_flags(pid_file);

    if (flags & CMD_FLAG_START)
        daemon_action_start(config, pid_file, process_flags);
    if (flags & CMD_FLAG_STOP)
        daemon_action_stop(pid_file, process_flags);
    if (flags & CMD_FLAG_RELOAD)
        daemon_action_reload(pid_file, process_flags);
    if (flags & CMD_FLAG_RESTART)
        daemon_action_restart(config, pid_file, process_flags);

    exit(EXIT_SUCCESS);
}

static pid_t read_pid_file(char *pid_file);

int retrieve_process_flags(char *pid_file)
{
    pid_t pid = read_pid_file(pid_file);
    if (pid == 0)
        return DAEMONS_NO_PIDFILE;

    int flags = 0;

    flags |= kill(pid, 0) == 0 ? DAEMONS_PROC_ALIVE : DAEMONS_PROC_DEAD;
    flags |= pid << 3;

    return flags;
}

pid_t read_pid_file(char *pid_file)
{
    FILE *file = fopen(pid_file, "r");
    if (file == NULL)
        return 0;

    char buffer[PID_STR_BUFF_SIZE];

    size_t num_read = 0;
    size_t total_num_read = 0;
    while ((num_read = fread(buffer + total_num_read, 1,
                             PID_STR_BUFF_SIZE - total_num_read - 1, file))
           != 0)
        total_num_read += num_read;

    buffer[total_num_read] = 0;
    return strtol(buffer, NULL, 10);
}
