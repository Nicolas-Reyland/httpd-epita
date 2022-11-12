#ifndef QUEUE_NODE_H
#define QUEUE_NODE_H

#include "multithreading/job/job.h"

struct queue_node
{
    struct queue_node *next;
    struct job job;
};

#endif /* !QUEUE_NODE_H */
