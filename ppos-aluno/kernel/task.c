// GRR20242288 Eduardo Munaretto Majczak
// GRR20242306 João Pedro Oliveira Lazari
// GRR20206889 Daniel Henrique Vieira
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica de tarefas.

#include <stdlib.h>

#include "task.h"
#include "../lib/queue.h"
#include "memory.h"
#include "ctx.h"

#define STACKSIZE 32 * 1024

int current_id = 0;
struct task_t *current_task = NULL;

void task_init() {
    current_id = 0;

    struct task_t *kernel = mem_alloc(sizeof(struct task_t));
    kernel->name = "kernel";
    kernel->id = 0;
    kernel->status = RODANDO;
    kernel->task_pai = NULL;

    current_task = kernel;
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
    task->status = PRONTA;
    if(ctx_create(&task->context, entry, arg, stack, STACKSIZE) == ERROR){
        mem_free(stack);
        mem_free(task);
        return NULL;
    }
    task->task_pai = current_task;
    task->vg_id = VALGRIND_STACK_REGISTER(task->context.stack, task->context.stack + STACKSIZE);

    return task;
}

int task_destroy(struct task_t *task) {
    if (task == NULL) {
        return ERROR;
    }

    task->task_pai = NULL;
    task->status = TERMINADA;
    if (task->id)
        free(task->context.stack);
    VALGRIND_STACK_DEREGISTER (task->vg_id);
    free(task);

    return NOERROR;
}

int task_switch(struct task_t *task) {
    struct task_t * task_switch;
    struct task_t * previous_task;

    if (task == NULL) {
        if (!current_task || !current_task->task_pai) {
            return ERROR;
        }
        
        task_switch = current_task->task_pai;
    } else {
        task_switch = task;
    }
    previous_task = current_task;

    if (previous_task)
        previous_task->status = SUSPENSA;

    current_task = task_switch;
    if (current_task)
        current_task->status = RODANDO;

    if (ctx_swap(previous_task ? &previous_task->context : NULL, task_switch ? &task_switch->context : NULL) == ERROR)
        return ERROR;

    return NOERROR;
}

int task_id(struct task_t *task) {
    if (task == NULL) {
        if (current_task)
            return current_task->id;
        else 
            return ERROR;
    }
    return task->id;
}

char *task_name(struct task_t *task) {
    if (task == NULL) {
        if (current_task && current_task->name)
            return current_task->name;
        else
            return NULL;
    } else if (task->name == NULL) {
        return NULL;
    }
    return task->name;
}