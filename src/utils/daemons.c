#include "daemons.h"

#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils/arg_flags.h"
#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"

#define PID_STR_BUFF_SIZE 1024

static int retrieve_process_info(char *pid_file);

static void write_own_pid(char *pid_file);

_Noreturn void handle_daemon(struct server_config *config, int flags)
{
    char *pid_file = hash_map_get(config->global, "pid_file");
    if (pid_file == NULL)
    {
        log_error("%s: no pid file in server config\n", __func__);
        exit(EXIT_FAILURE);
    }
    int process_flags = retrieve_process_info(pid_file);
    pid_t pid = (process_flags & DAEMONS_PID_MASK) >> 3;

    if (flags & CMD_FLAG_START)
    {
        if (process_flags & DAEMONS_PROC_ALIVE)
            kill(pid, SIGINT);

        write_own_pid(pid_file);
        start_all(config);
    }
    if (flags & CMD_FLAG_STOP)
    {
        kill(pid, SIGTERM);
    }
    if (flags & CMD_FLAG_RELOAD)
    {}
    if (flags & CMD_FLAG_RESTART)
    {}

    // daemonize self
    daemon(1, 1);

    exit(EXIT_SUCCESS);
}

static pid_t read_pid_file(char *pid_file);

int retrieve_process_info(char *pid_file)
{
    // readable file (and exists) ?
    if (access(pid_file, R_OK) != 0)
        return DAEMONS_NO_PIDFILE;

    pid_t pid = read_pid_file(pid_file);
    int flags = 0;

    flags |= kill(pid, 0) == 0 ? DAEMONS_PROC_ALIVE : DAEMONS_PROC_DEAD;
    flags |= pid << 3;

    return flags;
}

pid_t read_pid_file(char *pid_file)
{
    FILE *file = fopen(pid_file, "r");
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

void write_own_pid(char *pid_file)
{
    //
}
