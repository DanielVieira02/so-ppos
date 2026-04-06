// GRR20242288 Eduardo Munaretto Majczak
// GRR20242306 João Pedro Oliveira Lazari
// GRR20206889 Daniel Henrique Vieira
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

#include <stdio.h>

#include "../lib/queue.h"

#include "task.h"


void user_main(void *arg); 

//variavel global do dispatcher
struct task_t *dispatcher_task;
struct queue_t *ready_queue;

//ponteiro para a tarefa atual
extern struct task_t *current_task;

extern int userTasks;

void dispatcher_init() {
    ready_queue = queue_create();
    task_init();
    dispatcher_task = current_task;
}

void task_run(struct task_t *task){

    //retira a tarefa task da fila de prontas
    if(queue_del(ready_queue, (void *) task) == 0){
        task->status = EXECUTING;

        if(task_switch(task) < 0)
            printf("ERRO na transferencia de CPU");
    }
    else {
            printf("ERRO: Tarefa nao encontrada na fila de prontas \n");
        }

}

void task_yield(){
    current_task->status = READY;
    //tarefa atual esta pronta
    queue_add(ready_queue, (void*) current_task );
    task_switch(dispatcher_task);

}

void task_suspend(struct queue_t *queue){
    current_task->status = SUSPENDED;
    //tarefa atual esta suspensa
    if(current_task){
        queue_add(queue, (void *) current_task);
        current_task->suspend_queue = queue;
    }
    task_switch(dispatcher_task);
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
    current_task->status = DONE;
    task_switch(dispatcher_task);
}

void dispatcher() {
    struct task_t * task_user;

    task_user = task_create("user", user_main, 0);

    while(userTasks > 0) {
        //Scheduler provisorio
        struct task_t *next = NULL;

        //scheduler pega o item do primeiro no
        if (ready_queue != NULL && (queue_size(ready_queue) > 0))
            next = (struct task_t *) queue_head(ready_queue);

        if(next != NULL){
            task_run(next);

            //a tarefa next acabou de terminar de rodar
            switch(next->status){
                case READY:
                //se ela deu yield, ja foi inserida na fila em ready_queue
                break;

                case DONE:
                task_destroy(next);
                //limpar contexto, liberar memoria e decrementar userTasks
                userTasks--;
                break;

                case SUSPENDED:
                    break;
            }

        
        }
    }
    task_exit(0);
}
