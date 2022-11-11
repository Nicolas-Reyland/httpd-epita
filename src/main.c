#ifndef CUSTOM_MAIN

#    include <stdbool.h>
#    include <stdio.h>
#    include <string.h>

#    include "network/server.h"
#    include "process/daemons.h"
#    include "utils/arg_flags.h"
#    include "utils/hash_map/hash_map.h"
#    include "utils/logging.h"
#    include "utils/parsers/config/config_parser.h"
#    include "utils/state.h"

static char *read_cmd_args(int argc, char **argv, int *error);

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

    if (flags & CMD_FLAG_DAEMON)
        handle_daemon(config, flags);

    start_all(config, NULL);
}

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

#endif /* !CUSTOM_MAIN */
