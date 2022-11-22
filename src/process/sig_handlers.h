#ifndef SIG_HANDLERS_H
#define SIG_HANDLERS_H

#include <stdnoreturn.h>

void signal_handler(int signum);

_Noreturn void graceful_shutdown(void);

#endif /* !SIG_HANDLERS_H */
