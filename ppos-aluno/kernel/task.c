// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.

#include <stdlib.h>

#include "task.h"
#include "../lib/queue.h"

#define STACKSIZE 32 * 1024

struct queue_t * task_queue;

int current_id;
struct task_t *current_task;

void task_init() {
    current_id = 0;

    task_queue = queue_create();
    struct task_t *kernel_task = task_create("task_kernel", NULL, NULL);
    queue_add(task_queue, kernel_task);
    current_task = (struct task_t *)queue_head(task_queue);
}

struct task_t *task_create(char *name, void (*entry)(void *), void *arg) {
    struct task_t *task;

    task = malloc(sizeof(struct task_t));

    if(task == NULL) {
        return NULL;
    }

    char * stack;
    stack = malloc(STACKSIZE);

    if(stack == NULL) {
        return NULL;
    }

    task->id = current_id;
    task->name = name;
    task->status = READY;
    if(ctx_create(&task->context, entry, arg, stack, STACKSIZE) == ERROR){
        return NULL;
    }

    current_id++;
    queue_add(task_queue, task);

    return task;
}

int task_destroy(struct task_t *task) {
    if (task == NULL) {
        return ERROR;
    }

    queue_del(task_queue, task);

    free(task->context.stack);
    free(task);

    return NOERROR;
}

int task_switch(struct task_t *task) {
    if (task == NULL) {
        return ERROR;
    }

    current_task->status = READY;
    ctx_swap(&current_task->context, &task->context);
    current_task = task;
    current_task->status = EXECUTING;
}

int task_id(struct task_t *task) {
    if (task == NULL) {
        return current_task->id;
    }
    return task->id;
}

char *task_name(struct task_t *task) {
    if (task == NULL) {
        return current_id;
    }
}