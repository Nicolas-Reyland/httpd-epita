#ifndef STATE_H
#define STATE_H

#include "network/server_env.h"
#include "utils/logging.h"

#ifndef LOG_LEVEL
#    define LOG_LEVEL LOG_EPITA
#endif /* !LOG_LEVEL */

struct state;

extern struct state g_state;

struct state
{
    enum log_level log_level;
    struct server_env *env;
};

void setup_g_state(struct server_env *env);

#endif /* !STATE_H */