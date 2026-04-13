// GRR20242288 Eduardo Munaretto Majczak
// GRR20242306 João Pedro Oliveira Lazari
// GRR20206889 Daniel Henrique Vieira
// PingPongOS - PingPong Operating System

// Escalonador de tarefas prontas.
#include "../lib/queue.h"
#include "task.h"

extern struct task_t *current_task;

void sched_init()
{
}

void sched_setprio(struct task_t *task, int prio){
    if(prio < -20 || prio > 20)
        return;

    if(!task){
        current_task->static_priority = prio;
        current_task->dynamic_priority = prio;
        return;
    }

    task->static_priority = prio;
    task->dynamic_priority = prio;
}

int sched_getprio(struct task_t *task){
    if(!task){
        return current_task->static_priority;
    }

    return task->static_priority;
}

//uma função scheduler que analisa a fila de tarefas prontas e devolve um ponteiro para a próxima tarefa a receber o processador.
struct task_t *scheduler(struct queue_t *ready_queue){
    int tamanho = queue_size(ready_queue);
    struct task_t *next_task = queue_head(ready_queue);
    struct task_t *aux = next_task;

    //testa toda a fila em busca de alguem com menor prioridade dinamica
    for(int i = 0; i < tamanho; i++){
        if(aux->dynamic_priority < next_task->dynamic_priority){
            next_task = aux;
        }

        aux = queue_next(ready_queue);
    }

    aux = queue_head(ready_queue);
    for(int i = 0; i<tamanho; i++){
        if(aux != next_task) {
            //diminui a prioridade de quem ficou esperando
            if(aux->dynamic_priority > -20) {
                aux->dynamic_priority -=1;
            }
        } else {
            aux->dynamic_priority = aux->static_priority;
        }
        aux = queue_next(ready_queue);
    }

    return next_task;
}



