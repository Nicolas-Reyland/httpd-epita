#include "state.h"

struct state g_state = { 0 };

void setup_g_state(struct server_env *env)
{
    g_state.log_level = LOG_LEVEL;
    g_state.env = env;
}
