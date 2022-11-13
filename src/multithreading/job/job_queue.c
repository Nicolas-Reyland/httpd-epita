#include "job_queue.h"

#include <stdio.h>
#include <stdlib.h>

#include "multithreading/job/job_queue.h"
#include "utils/logging.h"
#include "utils/mem.h"

struct job_queue *job_queue_init(void)
{
    struct job_queue *queue = malloc(sizeof(struct job_queue));
    if (queue == NULL)
    {
        log_error("%s: Out of memory\n", __func__);
        return NULL;
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;

    return queue;
}

void job_queue_destroy(struct job_queue *job_queue)
{
    if (job_queue == NULL)
        return;

    struct queue_node *head = job_queue->head;
    while (head != NULL)
    {
        struct queue_node *prev = head;
        head = head->next;
        FREE_SET_NULL(prev);
    }
    FREE_SET_NULL(job_queue);
}

size_t job_queue_size(struct job_queue *job_queue)
{
    return job_queue->size;
}

/*
 * Returns the size of the queue on success, -1 on error (OOM)
 */
int job_queue_push(struct job_queue *job_queue, struct job job)
{
    if (job_queue == NULL)
        return -1;

    struct queue_node *new_tail = malloc(sizeof(struct queue_node));
    if (new_tail == NULL)
        return -1;

    new_tail->job = job;
    new_tail->next = NULL;

    // empty queue
    if (job_queue->head == NULL)
        job_queue->head = new_tail;
    // one element in queue
    else if (job_queue->tail == NULL)
    {
        job_queue->tail = new_tail;
        job_queue->head->next = new_tail;
    }
    // at least two elements in queue
    else
    {
        job_queue->tail->next = new_tail;
        job_queue->tail = new_tail;
    }

    return ++job_queue->size;
}

struct job job_queue_head(struct job_queue *job_queue)
{
    struct job job_idle = { .type = JOB_IDLE };
    if (job_queue == NULL || job_queue->size == 0)
        return job_idle;

    return job_queue->head->job;
}

void job_queue_pop(struct job_queue *job_queue, struct job *job)
{
    struct job job_idle = { .type = JOB_IDLE };
    *job = job_idle;

    if (job_queue == NULL || job_queue->head == NULL)
        return;

    // Set the value
    *job = job_queue_head(job_queue);

    if (job_queue_size(job_queue) == 1)
    {
        job_queue_clear(job_queue);
        return;
    }
    struct queue_node *old_head = job_queue->head;
    struct queue_node *new_head = job_queue->head->next;

    job_queue->head = new_head;
    --job_queue->size;
    free(old_head);
}

void job_queue_clear(struct job_queue *job_queue)
{
    struct queue_node *head = job_queue->head;
    while (head != NULL)
    {
        struct queue_node *tmp = head;
        head = head->next;
        free(tmp);
    }

    job_queue->head = NULL;
    job_queue->tail = NULL;
    job_queue->size = 0;
}
