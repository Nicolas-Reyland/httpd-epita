#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

#include "utils/queue/node.h"

struct queue
{
    struct queue_node *head;
    struct queue_node *tail;
    size_t size;
};

struct queue *queue_init(void);

void queue_destroy(struct queue *queue);

size_t queue_size(struct queue *queue);

int queue_push(struct queue *queue, void *data);

void *queue_head(struct queue *queue);

void *queue_pop(struct queue *queue);

void queue_clear(struct queue *queue, bool free_data);

#endif /* !JOB_QUEUE_H */
