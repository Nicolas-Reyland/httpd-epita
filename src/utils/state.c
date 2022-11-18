#include "state.h"

#include <string.h>

#include "process/sig_handlers.h"
#include "utils/hash_map/hash_map.h"

struct state g_state = {
    .env = NULL,
    .log_level = LOG_LEVEL,
    .logging = false,
    .log_file_stream = NULL,
};

int setup_g_state(struct server_env *env)
{
    // Server Environment
    g_state.env = env;

    // Logging
    g_state.log_level = LOG_LEVEL;
    if (g_state.logging)
    {
        char *log_file_path = hash_map_get(env->config->global, "log_file");
        g_state.log_file_stream = NULL;
        // choose right stream to write logs to
        if (log_file_path == NULL || strcmp(log_file_path, "stdout") == 0)
            g_state.log_file_stream = stdout;
        else if (strcmp(log_file_path, "stderr") == 0)
            g_state.log_file_stream = stderr;
        else
        {
            g_state.log_file_stream = fopen(log_file_path, "w");
            // opening file failed
            if (g_state.log_file_stream == NULL)
            {
                log_error("%s: could not open log file \"%s\"\n", __func__,
                          log_file_path);
                graceful_shutdown();
            }
        }
    }

    return 0;
}

void set_g_state_logging(struct server_config *config)
{
    g_state.log_level = LOG_LEVEL;
    if (g_state.log_file_stream != NULL)
    {
        fclose(g_state.log_file_stream);
        g_state.log_file_stream = NULL;
    }
    char *log_value = hash_map_get(config->global, "log");
    g_state.logging = log_value == NULL ? true : strcmp(log_value, "true") == 0;
}
