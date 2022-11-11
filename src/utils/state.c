#include "state.h"

#include <string.h>

#include "utils/hash_map/hash_map.h"

struct state g_state = { 0 };

void setup_g_state(struct server_env *env)
{
    g_state.env = env;
    g_state.log_level = LOG_LEVEL;
    g_state.num_threads = NUM_THREADS;

    if (g_state.logging)
    {
        char *log_file_path = hash_map_get(env->config->global, "log_file");
        g_state.log_file =
            log_file_path == NULL ? stdout : fopen(log_file_path, "a");
    }
    else
        g_state.log_file = NULL;
}

void set_g_state_logging(struct server_config *config)
{
    char *log_value = hash_map_get(config->global, "log");
    g_state.logging = log_value == NULL ? true : strcmp(log_value, "true") == 0;
}
