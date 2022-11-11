#include "handlers.h"

#include <stdlib.h>
#include <unistd.h>

#include "network/server_env.h"
#include "network/vhost.h"
#include "utils/state.h"
_Noreturn void gracefull_shutdown(void)
{
    log_message(LOG_STDOUT | LOG_DEBUG, "Gracefully killing self...\n");

    free_server_env(g_state.env, true, true);
    exit(EXIT_SUCCESS);
}
