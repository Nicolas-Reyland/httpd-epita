#ifndef WORKER_H
#define WORKER_H

#include <pthread.h>

void start_worker(int socket_fd);

void wait_for_workers(void);

#endif /* !WORKER_H */
