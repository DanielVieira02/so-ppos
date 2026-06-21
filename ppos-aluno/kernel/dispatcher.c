// GRR20242288 Eduardo Munaretto Majczak
// GRR20242306 João Pedro Oliveira Lazari
// GRR20206889 Daniel Henrique Vieira
// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Dispatcher: gerencia os estados das tarefas.

#include <stdio.h>

#include "../lib/queue.h"
#include "../hardware/cpu.h"

#include "task.h"
#include "time.h"


#include "scheduler.h"

void user_main(void *arg); 

//variavel global do dispatcher
struct queue_t *ready_queue;
struct queue_t *sleep_queue;

//ponteiro para a tarefa atual
extern struct task_t *current_task;
extern struct task_t *task_kernel;
extern int aliveUserTasks;
extern int local_time;

void dispatcher_init() {
    ready_queue = queue_create();
    sleep_queue = queue_create(); // cria fila de tarefas adormecidas p7
}

void print_tasks(void * item) {
    printf("%d", ((struct task_t *) item)->id);
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

void task_suspend(struct queue_t *queue){
    if (!current_task || !queue) {
        return;
    }
    
    current_task->status = SUSPENDED;
    queue_add(queue, (void *) current_task);

    task_switch(task_kernel);
}

void task_awake(struct task_t *task){
    /** 
    * retira tarefa das filas de suspensao da atual: ao esperar por outra tarefa, 
    * a tarefa em wait eh imediatamente encerrada, ou seja, soh pode estar na fila da ultima tarefa que ela fez task_wait() 
    */
    queue_del(sleep_queue, (struct queue_t *) task);
    queue_del(current_task->suspend_queue, (struct queue_t *) task);
    task->task_wait = NULL;
    if(task->status == SUSPENDED){
        //ajusta o status de task tarefa para “pronta”;
        task->status = READY;

        //insere task na fila de tarefas prontas;
        queue_add(ready_queue, (void *) task);
    }
}

void task_exit(int exit_code) {
    current_task->status = DONE;
    current_task->alive_time = systime() - current_task->birth_time;
    current_task->exit_code = exit_code;

    void *queue_task = queue_head(current_task->suspend_queue);

    while(queue_task != NULL) {
        task_awake((struct task_t *) queue_task);
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
    current_task->task_wait = task;
    task_suspend(task->suspend_queue);
    return task->exit_code;
}

void task_sleep(int t){
    if (t > 0) {
        current_task->wake_time = systime() + t;
        current_task->status = SUSPENDED;
        
        task_suspend(sleep_queue);
    }
}

void check_sleeping_tasks() {
    if (queue_size(sleep_queue) <= 0) {
        return;
    }

    int agora = systime();

    struct task_t *t = queue_head(sleep_queue);
    while(t != NULL){
        struct task_t *aux = t;
        t = queue_next(sleep_queue);

        if(agora >= aux->wake_time) {
            task_awake(aux);
        }
    }
}

void dispatcher() {
    struct task_t * task_user;
    if (!(task_user = task_create("user", user_main, NULL))) {
        return;
    };
    while(aliveUserTasks) {
        check_sleeping_tasks();
        if (queue_size(ready_queue) <= 0) {
            hw_wfi();
        } else {
            struct task_t *next = NULL;
            task_kernel->number_activation++;

            //scheduler pega o item do primeiro no
            if (ready_queue != NULL && (queue_size(ready_queue) > 0))
                next = scheduler(ready_queue);

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
                        aliveUserTasks--;
                    break;

                    case SUSPENDED:
                        queue_del(ready_queue, next);
                        break;
                }
            }
        }
    }
    task_kernel->alive_time = systime() - task_kernel->birth_time;

    printf("PPOS: task %d (%s) exit code %d, %d ms elapsed time, %5d ms cpu time, %5d activations\n",
       task_kernel->id, task_kernel->name, 0, 
       task_kernel->alive_time, task_kernel->cpu_time, 
       task_kernel->number_activation);

    queue_destroy(ready_queue);
    queue_destroy(sleep_queue);
    task_destroy(task_user);
	task_destroy(task_kernel);
	current_task = NULL;
    ready_queue = NULL;
    sleep_queue = NULL;
	task_kernel = NULL;
}
