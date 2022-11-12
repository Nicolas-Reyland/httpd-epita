#ifndef DAEMONS_H
#define DAEMONS_H

#include <stdnoreturn.h>

#include "utils/parsers/config/config_parser.h"

#define DAEMONS_NO_PIDFILE 01
#define DAEMONS_PROC_ALIVE 02
#define DAEMONS_PROC_DEAD 004
#define DAEMONS_PID_MASK ~007

_Noreturn void handle_daemon(struct server_config *config, int flags);

#endif /* !DAEMONS_H */
