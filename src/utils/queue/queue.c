#include "queue.h"

#include <stdio.h>
#include <stdlib.h>

struct queue *queue_init(void)
{
    struct queue *queue = malloc(sizeof(struct queue));
    if (queue == NULL)
        return NULL;

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;

    return queue;
}

void queue_destroy(struct queue *queue)
{
    if (queue == NULL)
        return;

    struct queue_node *head = queue->head;
    while (head != NULL)
    {
        free(head->data);
        head->data = NULL;

        struct queue_node *prev = head;
        head = head->next;
        free(prev);
        prev = NULL;
    }
    free(queue);
    queue = NULL;
}

size_t queue_size(struct queue *queue)
{
    return queue->size;
}

/*
 * Returns the size of the queue on success, -1 on error (OOM)
 */
int queue_push(struct queue *queue, void *data)
{
    if (queue == NULL)
        return -1;

    struct queue_node *new_tail = malloc(sizeof(struct queue_node));
    if (new_tail == NULL)
        return -1;

    new_tail->data = data;
    new_tail->next = NULL;

    // empty queue
    if (queue->head == NULL)
        queue->head = new_tail;
    // one element in queue
    else if (queue->tail == NULL)
    {
        queue->tail = new_tail;
        queue->head->next = new_tail;
    }
    // at least two elements in queue
    else
    {
        queue->tail->next = new_tail;
        queue->tail = new_tail;
    }

    return ++queue->size;
}

void *queue_head(struct queue *queue)
{
    if (queue == NULL || queue->size == 0)
        return NULL;

    return queue->head->data;
}

void *queue_pop(struct queue *queue)
{
    if (queue == NULL || queue->head == NULL)
        return NULL;

    void *value = queue_head(queue);

    if (queue_size(queue) == 1)
        queue_clear(queue, false);
    else
    {
        struct queue_node *old_head = queue->head;
        struct queue_node *new_head = queue->head->next;

        queue->head = new_head;
        --queue->size;
        free(old_head);
    }

    return value;
}

void queue_clear(struct queue *queue, bool free_data)
{
    struct queue_node *head = queue->head;
    while (head != NULL)
    {
        if (free_data)
        {
            free(head->data);
            head->data = NULL;
        }
        struct queue_node *tmp = head;
        head = head->next;

        free(tmp);
    }

    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
}
