#ifndef STATE_H
#define STATE_H

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

#include "network/server_env.h"
#include "utils/logging.h"
#include "utils/parsers/config/config_parser.h"
#include "utils/queue/queue.h"

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
    // Server Environment
    struct server_env *env;
    // Logging
    enum log_level log_level;
    bool logging;
    FILE *log_file_stream;
    // Multi-threading
    size_t max_num_threads;
    pthread_t *thread_ids;
    size_t num_active_threads;
    struct queue *terminated_workers;
    pthread_mutex_t threads_mutex;
    int default_lock_timeout;
    // Job queues
    struct queue **job_queues;
    pthread_mutex_t *job_queues_mutexes;
};

int setup_g_state(struct server_env *env);

void set_g_state_logging(struct server_config *config);

#endif /* !STATE_H */
