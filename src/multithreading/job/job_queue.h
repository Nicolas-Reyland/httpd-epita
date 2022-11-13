#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <stddef.h>

#include "multithreading/job/queue_node.h"

struct job_queue
{
    struct queue_node *head;
    struct queue_node *tail;
    size_t size;
};

struct job_queue *job_queue_init(void);

void job_queue_destroy(struct job_queue *job_queue);

size_t job_queue_size(struct job_queue *job_queue);

int job_queue_push(struct job_queue *job_queue, struct job job);

struct job job_queue_head(struct job_queue *job_queue);

void job_queue_pop(struct job_queue *job_queue, struct job *job);

void job_queue_clear(struct job_queue *job_queue);

#endif /* !JOB_QUEUE_H */
