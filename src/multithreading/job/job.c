#include "job.h"

#include <pthread.h>
#include <string.h>

#include "multithreading/mutex_wrappers.h"
#include "network/client.h"
#include "network/socket_handler.h"
#include "utils/logging.h"
#include "utils/socket_utils.h"
#include "utils/state.h"

void add_job_to_queue(struct job *job)
{
    static size_t job_uid = 1;

    if (job == NULL)
    {
        log_warn("%s: trying to push nulll job to queue ? push cancelled\n",
                 __func__);
        return;
    }
    job->uid = job_uid++;

    log_debug("Adding job {type = %d, socket_fd = %d, index = %zd, uid = %zu} "
              "to queue\n",
              job->type, job->socket_fd, job->index, job->uid);

    {
        int error;
        if ((error = lock_mutex_wrapper(&g_state.job_queue_mutex)))
        {
            log_error("%s(lock job queue): %s\n", __func__, strerror(error));
            return;
        }
    }

    if (job->type == JOB_IDLE)
        log_warn("%s: pushing idle job (uid = %zu) to queue\n", __func__,
                 job->uid);

    if (queue_push(g_state.job_queue, job) == -1)
        log_error("%s: failed to push job of type %d to queue\n", __func__,
                  job->type);

    if (g_state.job_queue->size == 1)
        log_debug("%s: there is 1 job waiting in queue\n", __func__);
    else
        log_debug("%s: there are %zu jobs waiting in queue\n", __func__,
                  g_state.job_queue->size);

    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.job_queue_mutex)))
        {
            log_error("%s(unlock job queue): %s\n", __func__, strerror(error));
            return;
        }
    }
}

static void execute_accept_job(struct job *job);

static void execute_process_job(struct job *job);

static void execute_close_job(struct job *job);

void execute_job(struct job *job)
{
    log_info("Executing job (uid = %zu)\n", job->uid);

    switch (job->type)
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
        log_warn("%s: unkown job type %d\n", __func__, job->type);
        break;
    }
}

void execute_accept_job(struct job *job)
{
    log_debug("Executing access job (uid = %zu)\n", job->uid);
    register_connection(job->socket_fd);

    free(job);
    job = NULL;
}

static int lock_vector_containing_client(struct client *client);

void execute_process_job(struct job *job)
{
    log_debug("Executing process job (uid = %zu)\n", job->uid);

    // should wait for vhost clients
    struct client *client = client_from_client_socket(job->socket_fd, true);
    if (client == NULL)
    {
        log_error(
            "[%d] %s(lock client): could not find a client associated to %d\n",
            pthread_self(), __func__, job->socket_fd);

        free(job);
        job = NULL;
        return;
    }

    // work
    bool alive;
    size_t data_len;
    char *data = read_from_connection(client->socket_fd, &data_len, &alive);
    if (!alive)
    {
        log_debug("Closing dead connection while processing job (uid = %zu)\n",
                  job->uid);
        // close the connection now : why bother adding the task to another
        // thread ? the client is already locked, so let's do it now

        /*
        struct vhost *vhost = client->vhost;
        if (lock_vector_containing_client(client) == -1)
        {
            */
        log_warn("[%d] %s: locking vector from client failed. manually "
                 "adding close job to queue\n",
                 pthread_self(), __func__);
        struct job close_job = {
            .type = JOB_CLOSE,
            .socket_fd = client->socket_fd,
            .index = client->index,
        };
        *job = close_job;
        add_job_to_queue(job);

        log_debug("[%d] Finished (early) process job (uid = %zu)\n",
                  pthread_self(), job->uid);
        return;
        /*
                }
                else
                {
                    log_debug("[%d] %s: (job uid = %zu) loced vector for
           client %d\n", pthread_self(), __func__, job->uid,
           job->socket_fd);

                    close_connection(client);
                    int error;
                    if ((error =
           pthread_mutex_unlock(&vhost->clients_mutex))) log_error("[%d]
           %s(manual close unlock vector, ignore): %s\n", pthread_self(),
           __func__, strerror(error));
                }*/
    }
    else
    {
        process_data(client, data, data_len);
        free(data);

        // unlock client and return
        release_client(client);
    }

    log_debug("[%d] Finished process job (uid = %zu)\n", pthread_self(),
              job->uid);

    free(job);
    job = NULL;
}

void execute_close_job(struct job *job)
{
    log_debug("Executing close job (uid = %zu)\n", job->uid);

    // lock client
    struct client *client = client_from_client_socket(job->socket_fd, false);
    if (client == NULL)
    {
        log_error("[%d] %s(lock client): could not find a client "
                  "associated to %d\n",
                  pthread_self(), __func__, job->socket_fd);

        free(job);
        job = NULL;
        return;
    }

    // lock vector containing client (which is locked)
    if (lock_vector_containing_client(client) == -1)
    {
        log_error("[%d] %s(lock vector client): failed to lock vector "
                  "associated to %d\n",
                  pthread_self(), __func__, job->socket_fd);
        release_client(client);

        free(job);
        job = NULL;
        return;
    }

    // work
    struct vhost *vhost = client->vhost;
    close_connection(client);

    // unlock vector (NOT the client, since it has been removed)
    {
        int error;
        if ((error = pthread_mutex_unlock(&vhost->clients_mutex)))
        {
            log_error("[%d] %s(unlock clients vector, ignored): %s\n",
                      pthread_self(), __func__, strerror(error));
        }
    }

    free(job);
    job = NULL;
}

/*
 * client MUST be locked
 */
int lock_vector_containing_client(struct client *client)
{
    // check for vhost existence
    if (client->vhost == NULL)
    {
        log_error("[%d] %s(client vhost check): no vhost associated to the "
                  "client %d\n",
                  pthread_self(), __func__, client->socket_fd);
        return -1;
    }
    // lock vector in which the client resides
    {
        int error;
        if ((error = lock_mutex_wrapper(&client->vhost->clients_mutex)))
        {
            log_error("[%d] %s(lock clients vector): %s\n", pthread_self(),
                      __func__, strerror(error));
            return -1;
        }
    }

    return 0;
}

void join_terminated_workers(void)
{
    {
        int error;
        if ((error = lock_mutex_wrapper(&g_state.threads_mutex)))
        {
            log_error("%s(lock threads mutex): %s\n", __func__,
                      strerror(error));
            return;
        }
    }

    pthread_t *thread_id;
    while ((thread_id = queue_pop(g_state.terminated_workers)) != NULL)
    {
        log_info("%s: Joining thread %d\n", __func__, *thread_id);
        pthread_join(*thread_id, NULL);
        free(thread_id);
        thread_id = NULL;
    }

    {
        int error;
        if ((error = pthread_mutex_unlock(&g_state.threads_mutex)))
        {
            log_error("%s(unlock threads mutex): %s\n", __func__,
                      strerror(error));
            return;
        }
    }

    log_debug("%s: Done joining terminated workers\n", __func__);
}
