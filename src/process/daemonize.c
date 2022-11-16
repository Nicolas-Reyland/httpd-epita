#include "daemonize.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils/logging.h"

void daemonize(void)
{
    pid_t cpid = fork();
    if (cpid < 0)
    {
        log_error("%s(first fork): %s\n", __func__, strerror(errno));
        exit(1);
    }

    if (cpid > 0)
        exit(0);

    pid_t sid = setsid();
    if (sid < 0)
    {
        log_error("%s(setsid(: %s\n", __func__, strerror(errno));
        exit(1);
    }

    cpid = fork();
    if (cpid < 0)
    {
        log_error("%s(first fork): %s\n", __func__, strerror(errno));
        exit(1);
    }

    if (cpid > 0)
        exit(0);

    log_info("Started daemon process with pid %d\n", getpid());
}
