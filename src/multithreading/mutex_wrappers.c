#include "mutex_wrappers.h"

#include <string.h>
#include <time.h>

#include "utils/logging.h"
#include "utils/state.h"

int lock_mutex_wrapper(pthread_mutex_t *mutex)
{
#ifdef MUTEX_TIMED_LOCKS
    struct timespec timeout_spec;
    clock_gettime(CLOCK_REALTIME, &timeout_spec);
    timeout_spec.tv_sec += g_state.default_lock_timeout;
    return pthread_mutex_timedlock(mutex, &timeout_spec);
#else
    return pthread_mutex_lock(mutex);
#endif /* MUTEX_TIMED_LOCKS */
}

int init_mutex_wrapper(pthread_mutex_t *mutex)
{
    pthread_mutexattr_t mutex_attr;

    int error;
    if ((error = pthread_mutexattr_init(&mutex_attr)))
    {
        log_error("[%d] %s(generic mutex attr init): %s\n", pthread_self(),
                  __func__, strerror(error));
        return error;
    }
    if ((error =
             pthread_mutexattr_setrobust(&mutex_attr, PTHREAD_MUTEX_ROBUST)))
    {
        log_error("[%d] %s(generic mutex attr setrobust): %s\n", pthread_self(),
                  __func__, strerror(error));
        return error;
    }

    return pthread_mutex_init(mutex, &mutex_attr);
}
