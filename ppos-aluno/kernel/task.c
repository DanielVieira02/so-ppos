// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.

#include <stdlib.h>

#include "task.h"
#include "../lib/queue.h"
#include "memory.h"

#define STACKSIZE 32 * 1024

int current_id = 0;
struct task_t *current_task = NULL;

void task_init() {
    current_id = 0;

    struct task_t *task_kernel = mem_alloc(sizeof(struct task_t));
    task_kernel->name = 'kernel';
    task_kernel->id = 0;
    task_kernel->status = EXECUTING;
    task_kernel->task_pai = NULL;

    current_task = task_kernel;
}

struct task_t *task_create(char *name, void (*entry)(void *), void *arg) {
    struct task_t *task;

    task = mem_alloc(sizeof(struct task_t));

    if(task == NULL) {
        return NULL;
    }

    void * stack;
    stack = (void *) mem_alloc(STACKSIZE);

    if(stack == NULL) {
        return NULL;
    }

    task->id = ++current_id;
    task->name = name;
    task->status = READY;
    if(ctx_create(&task->context, entry, arg, stack, STACKSIZE) == ERROR){
        mem_free(stack);
        mem_free(task);
        return NULL;
    }
    task->task_pai = current_task;

    return task;
}

int task_destroy(struct task_t *task) {
    if (task == NULL) {
        return ERROR;
    }

    free(task);
    task->task_pai = NULL;
    task->status = DONE;

    return NOERROR;
}

int task_switch(struct task_t *task) {
    struct task_t * task_switch;

    if (task == NULL) {
        if (!current_task || !current_task->task_pai)
            return ERROR;

        task_switch = current_task->task_pai;
    } else {
        task_switch = task;
    }

    current_task->status = READY;
    if (ctx_swap(&current_task->context, &task_switch->context) == ERROR)
        return ERROR;

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