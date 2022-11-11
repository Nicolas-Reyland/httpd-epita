#ifndef STATE_H
#define STATE_H

#include <stdio.h>

#include "network/server_env.h"
#include "utils/logging.h"
#include "utils/parsers/config/config_parser.h"

#ifndef LOG_LEVEL
#    define LOG_LEVEL LOG_EPITA
#endif /* !LOG_LEVEL */

#ifndef NUM_THREADS
#    define NUM_THREADS 3
#endif /* !NUM_THREADS */

struct state;

extern struct state g_state;

struct state
{
    enum log_level log_level;
    struct server_env *env;
    int num_threads;
    bool logging;
    FILE *log_file;
};

void setup_g_state(struct server_env *env);

void set_g_state_logging(struct server_config *config);

#endif /* !STATE_H */
