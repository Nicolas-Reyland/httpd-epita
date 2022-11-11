#include <stdbool.h>
#include <stdio.h>
#include <stdnoreturn.h>
#include <string.h>

#include "network/server.h"
#include "utils/hash_map/hash_map.h"
#include "utils/logging.h"
#include "utils/parsers/config/config_parser.h"
#include "utils/state.h"

#define CMD_FLAG_DRY_RUN 0001
#define CMD_FLAG_DAEMON 00002
#define CMD_FLAG_START 000004
#define CMD_FLAG_STOP 0000010
#define CMD_FLAG_RELOAD 00020
#define CMD_FLAG_RESTART 0040
#define CMD_FLAG_INVALID 0100

static char *read_cmd_args(int argc, char **argv, int *error);

#ifndef CUSTOM_MAIN
int main(int argc, char **argv)
{
    int flags;
    char *configfile = read_cmd_args(argc, argv, &flags);
    if (flags & CMD_FLAG_INVALID)
    {
        log_error("Usage: ./httpd [--dry-run] [-a (start | stop | reload | "
                  "restart)] server.conf\n");
        exit(EXIT_FAILURE);
    }

    struct server_config *config = parse_server_config(configfile);
    config = fill_server_config(config);

    // dry-run edge-case
    if (flags & CMD_FLAG_DRY_RUN)
        exit(config == NULL ? 2 : EXIT_SUCCESS);

    if (config == NULL)
    {
        log_error("Invalid config file\n");
        exit(EXIT_FAILURE);
    }

    start_all(1, config);
}
#endif /* !CUSTOM_MAIN */

static int daemon_flag_from_string(char *arg);

char *read_cmd_args(int argc, char **argv, int *flags)
{
    *flags = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "--dry-run") == 0)
        {
            *flags |= CMD_FLAG_DRY_RUN;
            // --dry-run must be the only arguments
            if (i != 1 || argc != 3)
                *flags |= CMD_FLAG_INVALID;
            return argv[i + 1];
        }
        if (strcmp(argv[i], "-a") == 0)
        {
            *flags |= CMD_FLAG_DAEMON | daemon_flag_from_string(argv[++i]);
            continue;
        }
        // Assuming the last argument is the config file
        if (argv[i + 1] != NULL)
            *flags |= CMD_FLAG_INVALID;
        return argv[i];
    }

    *flags |= CMD_FLAG_INVALID;
    return NULL;
}

int daemon_flag_from_string(char *arg)
{
    if (arg == NULL)
        return CMD_FLAG_INVALID;

    if (strcmp(arg, "start") == 0)
        return CMD_FLAG_START;
    if (strcmp(arg, "stop") == 0)
        return CMD_FLAG_STOP;
    if (strcmp(arg, "reload") == 0)
        return CMD_FLAG_RELOAD;
    if (strcmp(arg, "restart") == 0)
        return CMD_FLAG_RESTART;

    return CMD_FLAG_INVALID;
}
