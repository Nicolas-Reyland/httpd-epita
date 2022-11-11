#ifndef ACTIONS_H
#define ACTIONS_H

#include <stdnoreturn.h>

#include "utils/parsers/config/config_parser.h"

_Noreturn void daemon_action_start(struct server_config *config, char *pid_file,
                                   int process_flags);

_Noreturn void daemon_action_stop(char *pid_file, int process_flags);

_Noreturn void daemon_action_reload(char *pid_file, int process_flags);

_Noreturn void daemon_action_restart(struct server_config *config,
                                     char *pid_file, int process_flags);

#endif /* !ACTIONS_H */
