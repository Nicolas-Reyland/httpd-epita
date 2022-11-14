#ifndef MUTEX_WRAPPERS_H
#define MUTEX_WRAPPERS_H

#include <pthread.h>

int lock_mutex_wrapper(pthread_mutex_t *mutex);

int init_mutex_wrapper(pthread_mutex_t *mutex);

#endif /* !MUTEX_WRAPPERS_H */
