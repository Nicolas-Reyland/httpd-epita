#include "job.h"

#include <pthread.h>
#include <string.h>

#include "network/client.h"
#include "network/socket_handler.h"
#include "utils/logging.h"
#include "utils/socket_utils.h"
#include "utils/state.h"

void add_job_to_queue(struct job job)
{
    log_debug("Adding job of type %d to queue\n", job.type);

    {
        int error;
        if ((error = pthread_mutex_lock(&g_state.queue_mutex)))
        {
            log_error("%s(lock job queue): %s\n", __func__, strerror(error));
            return;
        }
    }

    if (job_queue_push(g_state.job_queue, job) == -1)
        log_error("%s: failed to push job of type %d to queue\n", __func__,
                  job.type);

    log_debug("%s: there are %zu jobs waiting in queue\n", __func__,
              g_state.job_queue->size);

    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.queue_mutex)))
        {
            log_error("%s(unlock job queue): %s\n", __func__, strerror(error));
            return;
        }
    }
}

static void execute_accept_job(struct job job);

static void execute_process_job(struct job job);

static void execute_close_job(struct job job);

void execute_job(struct job job)
{
    switch (job.type)
    {
    case JOB_ACCEPT:
        execute_accept_job(job);
        break;
    case JOB_PROCESS:
        execute_process_job(job);
        break;
    case JOB_CLOSE:
        execute_close_job(job);
        break;
    case JOB_IDLE:
        log_warn("%s: idle job passed down to execution\n", __func__);
        break;
    default:
        log_warn("%s: unkown job type %d\n", __func__, job.type);
        break;
    }
}

void execute_accept_job(struct job job)
{
    log_debug("Executing access job\n");
    register_connection(job.socket_fd);
}

void execute_process_job(struct job job)
{
    log_debug("Executing process job\n");

    struct client *client = client_from_client_socket(job.socket_fd, false);
    if (client == NULL)
    {
        log_error(
            "[%d] %s(lock client): could not find a client associated to %d\n",
            pthread_self(), __func__, job.socket_fd);
        return;
    }

    // work
    bool alive;
    size_t data_len;
    char *data = read_from_connection(client->socket_fd, &data_len, &alive);
    if (!alive)
    {
        // close the connection now : why bother adding the task to another
        // thread ? the client is already locked, so let's do it now
        close_connection(client->socket_fd);
    }
    else
    {
        // when threading, add (i, data, size) to queue instead of
        // doing it now, in the main loop
        process_data(client, data, data_len);
        free(data);
    }
    // unlock client and return
    {
        int error;
        if ((error = pthread_mutex_unlock(&client->mutex)))
        {
            log_debug("[%d] %s(unlock clients): %s\n", pthread_self(), __func__,
                      strerror(error));
        }
    }
}

void execute_close_job(struct job job)
{
    log_debug("Executing close job\n");

    // lock client
    struct client *client = client_from_client_socket(job.socket_fd, false);
    if (client == NULL)
    {
        log_error(
            "[%d] %s(lock client): could not find a client associated to %d\n",
            pthread_self(), __func__, job.socket_fd);
        return;
    }

    // work
    close_connection(client->socket_fd);

    // unlock client and return
    {
        int error;
        if ((error = pthread_mutex_unlock(&client->mutex)))
        {
            log_debug("[%d] %s(unlock clients): %s\n", pthread_self(), __func__,
                      strerror(error));
        }
    }
}
