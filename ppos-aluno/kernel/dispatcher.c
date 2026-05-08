// GRR20242288 Eduardo Munaretto Majczak
// GRR20242306 João Pedro Oliveira Lazari
// GRR20206889 Daniel Henrique Vieira
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

#include <stdio.h>

#include "../lib/queue.h"

#include "task.h"
#include "time.h"


#include "scheduler.h"

void user_main(void *arg); 

//variavel global do dispatcher
struct queue_t *ready_queue;

//ponteiro para a tarefa atual
extern struct task_t *current_task;
extern struct task_t *task_kernel;
extern int local_time;

void dispatcher_init() {
    ready_queue = queue_create();
}

void task_run(struct task_t *task) {

    //retira a tarefa task da fila de prontas
    if (queue_del(ready_queue, (void *) task) == 0) {
        task->status = EXECUTING;
        task->quantum = QUANTUM;
        task->number_activation++;

        if(task_switch(task) < 0) {
            printf("ERRO na transferencia de CPU\n");
        }
    } else {
            printf("ERRO: Tarefa nao encontrada na fila de prontas \n");
        }

}

void task_yield() {
    if (current_task->id) {
        //tarefa atual esta pronta
        queue_add(ready_queue, (void*) current_task);
        
        task_switch(task_kernel);
    }
}

void task_suspend(struct task_t *awaited_task){
    if (!current_task || !awaited_task) {
        return;
    }
    
    current_task->status = SUSPENDED;
    queue_add(awaited_task->suspend_queue, (void *) current_task);

    task_switch(task_kernel);
}

void task_awake(struct task_t *task){
    //se a tarefa task estiver suspensa em alguma fila, retira-a dessa fila;
    if(task->status == SUSPENDED){
        queue_del(task->suspend_queue, (struct queue_t *)task);
        task->suspend_queue = NULL;

        //ajusta o status de task tarefa para “pronta”;
        task->status = READY;

        //insere task na fila de tarefas prontas;
        queue_add(ready_queue, (void *)task);
    }
}

void task_exit(int exit_code) {
    current_task->status = DONE;
    current_task->alive_time = systime() - current_task->birth_time;
    current_task->exit_code = exit_code;

    void *queue_task = queue_head(current_task->suspend_queue);

    while(queue_task != NULL) {
        task_awake((struct task_t *)queue_task);
        queue_task = queue_next(current_task->suspend_queue);
    }

    printf("PPOS: task %d (%s) exit code %d, %5d ms elapsed time, %5d ms cpu time, %5d activations\n",
        current_task->id, current_task->name, exit_code, 
        current_task->alive_time, current_task->cpu_time, 
        current_task->number_activation);

    task_switch(task_kernel);
}

int task_wait(struct task_t *task) {
    if (task == NULL) {
        return -1;
    }
    if (task->status == DONE) {
        return task->exit_code;
    }

    task_suspend(task);
    return task->exit_code;
}

void dispatcher() {

    if (!task_create("user", user_main, NULL)) {
        return;
    };

    while(queue_size(ready_queue) > 0) {
        struct task_t *next = NULL;
        task_kernel->number_activation++;

        //scheduler pega o item do primeiro no
        if (ready_queue != NULL && (queue_size(ready_queue) > 0))
            next = scheduler(ready_queue);
            // next = queue_head(ready_queue);

        if(next != NULL){
            task_run(next);

            //a tarefa next acabou de terminar de rodar
            switch(next->status){
                case READY:
                    //se ela deu yield, ja foi inserida na fila em ready_queue
                break;

                case DONE:
                    //  limpar contexto, liberar memoria
                    queue_del(ready_queue, next);
                    //  task_destroy(next); a destruição as tarefas fica a cargo da tarefa que as criou
                break;

                case SUSPENDED:
                    queue_del(ready_queue, next);
                    break;
            }
        }
    }
    task_kernel->alive_time = systime() - task_kernel->birth_time;

    printf("PPOS: task %d (%s) exit code %d, %d ms elapsed time, %5d ms cpu time, %5d activations\n",
       task_kernel->id, task_kernel->name, 0, 
       task_kernel->alive_time, task_kernel->cpu_time, 
       task_kernel->number_activation);

	task_destroy(task_kernel);
	current_task = NULL;
	task_kernel = NULL;
    queue_destroy(ready_queue);
}
