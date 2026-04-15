// GRR20242288 Eduardo Munaretto Majczak
// GRR20242306 João Pedro Oliveira Lazari
// GRR20206889 Daniel Henrique Vieira
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

#include <stdio.h>

#include "../lib/queue.h"


#include "task.h"

#include "scheduler.h"

void user_main(void *arg); 

//variavel global do dispatcher
struct queue_t *ready_queue;

//ponteiro para a tarefa atual
extern struct task_t *current_task;
extern struct task_t *task_kernel;

void dispatcher_init() {
    ready_queue = queue_create();
}

void task_run(struct task_t *task) {

    //retira a tarefa task da fila de prontas
    if (queue_del(ready_queue, (void *) task) == 0) {
        task->status = EXECUTING;

        if(task_switch(task) < 0) {
            printf("ERRO na transferencia de CPU");
        }
    } else {
            printf("ERRO: Tarefa nao encontrada na fila de prontas \n");
        }

}

void task_yield() {
    current_task->status = READY;

    //tarefa atual esta pronta
    queue_add(ready_queue, (void*) current_task);

    task_switch(task_kernel);
}

void task_suspend(struct queue_t *queue){
    current_task->status = SUSPENDED;

    //tarefa atual esta suspensa
    if (current_task){
        queue_add(queue, (void *) current_task);
        current_task->suspend_queue = queue;
    }

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

void task_exit(int exit_code){
    if (current_task) {
        current_task->status = DONE;
    }

    task_switch(task_kernel);
}

void dispatcher() {
    struct task_t * task_user = task_create("user", user_main, NULL);
    queue_add(ready_queue, task_user);

    while(queue_size(ready_queue) > 0) {
        //Scheduler provisorio
        struct task_t *next = NULL;

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
                    task_destroy(next);
                break;

                case SUSPENDED:
                    queue_del(ready_queue, next);
                    break;
            }
        }
    }

    task_destroy(task_kernel);
    queue_destroy(ready_queue);
}
