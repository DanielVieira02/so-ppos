// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Escalonador de tarefas prontas.

#include "../lib/queue.h"

extern struct queue_t * ready_queue;

void sched_init()
{
}

struct task_t *scheduler(struct queue_t *ready_queue) {
    return queue_head(ready_queue);
};
