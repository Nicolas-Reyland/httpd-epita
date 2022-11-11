#include "arg_flags.h"

#include <stddef.h>
#include <string.h>

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
