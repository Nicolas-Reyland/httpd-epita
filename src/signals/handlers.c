#include "handlers.h"
#include <unistd.h>

#include <stdlib.h>

#include "network/vhost.h"
#include "network/server_env.h"
#include "utils/state.h"

#include <unistd.h>
_Noreturn void gracefull_shutdown(void)
{
    log_message(LOG_STDOUT | LOG_DEBUG, "Gracefully killing self...\n");

    struct server_env *env = g_state.env;
    free_server_env(env, true, true);

    exit(EXIT_SUCCESS);
}
