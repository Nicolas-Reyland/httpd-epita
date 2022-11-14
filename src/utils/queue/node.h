#ifndef QUEUE_NODE_H
#define QUEUE_NODE_H

struct queue_node
{
    struct queue_node *next;
    void *data;
};

#endif /* !QUEUE_NODE_H */
