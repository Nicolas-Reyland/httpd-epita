#ifndef SERVER_H
#define SERVER_H

#include <stdnoreturn.h>

#include "server_env.h"
#include "utils/parsers/config/config_parser.h"

_Noreturn void start_all(struct server_config *config);

_Noreturn void run_server(struct server_env *env);

#endif /* SERVER_H */
