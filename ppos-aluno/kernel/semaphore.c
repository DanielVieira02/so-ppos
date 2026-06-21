// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Semáforos e spinlocks

#include <stdio.h>
#include "semaphore.h"
#include "../lib/queue.h"
#include "../hardware/cpu.h"
#include "dispatcher.h"
#include "tcb.h"
#include "task.h"
#include "time.h"
#include "memory.h"

#define MAX_SEMAPHORES 64

extern struct task_t *current_task;
extern struct task_t *task_kernel;
extern struct queue_t *ready_queue;

struct semaphore_t{
    struct queue_t *fila;
    int contador;
    int lock;
};

// vetor de semaforos
static struct semaphore_t *tabela_semaforos[MAX_SEMAPHORES] = {NULL};

// Verifica se o ponteiro para o semaforo ainda eh valido
static int sem_existe(struct semaphore_t *s) {
    if (!s) return 0;
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (tabela_semaforos[i] == s) {
            return 1; 
        }
    }
    return 0;
}


void spin_lock(int *lock){
    while(__sync_lock_test_and_set(lock, 1)){
    }//le o valor antigo e seta 1
}

void spin_unlock(int *lock){
    __sync_lock_release(lock);
}

struct semaphore_t *sem_create(int value){
    struct semaphore_t *s = mem_alloc(sizeof(struct semaphore_t));
    if(!s){
        printf("Semaforo nao criado\n");
        return NULL;
    }

    s->contador= value;
    s->lock =0;
    s->fila = queue_create();

    if(!s->fila){
        printf("Fila do semaforo nao criada\n");
        mem_free(s);
        return NULL;
    }

    // REGISTRO: Insere o novo semáforo em um espaço vago da tabela
    int registrado = 0;
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (tabela_semaforos[i] == NULL) {
            tabela_semaforos[i] = s;
            registrado = 1;
            break;
        }
    }

    if (!registrado) {
        printf("Erro: Limite maximo de semaforos do SO atingido\n");
        queue_destroy(s->fila);
        mem_free(s);
        return NULL;
    }

    return s;

}

int sem_destroy(struct semaphore_t *s){
    if(!s || !s->fila){
        printf("Nao foi possivel destruir o semaforo\n");
        return ERROR;
    }

    hw_irq_enable(0);
    spin_lock(&s->lock);

    // Retira o semaforo da tabela global 
    for (int i = 0; i < MAX_SEMAPHORES; i++) {
        if (tabela_semaforos[i] == s) {
            tabela_semaforos[i] = NULL;
            break;
        }
    }

    struct task_t *t = queue_head(s->fila);
    while(t != NULL){
		    struct task_t *aux =t;
		    t = queue_next(s->fila);

		    queue_del(s->fila,aux);
		    aux->status = READY;
		    aux->exit_code = ERROR;
		    queue_add(ready_queue, aux);
        }

    queue_destroy(s->fila);
    hw_irq_enable(1);

    spin_unlock(&s->lock);

    mem_free(s);
    return NOERROR;
}

void sem_init()
{
}

int sem_down(struct semaphore_t *s){

    if(!sem_existe(s)){
        return ERROR;
    }

    if(!s || !s->fila){
        printf("Nao foi possivel fazer down no semaforo\n");
        return ERROR;
    }
    spin_lock(&s->lock);
    s->contador = s->contador -1;

    if(s->contador <0){
        current_task->status = SUSPENDED;

        hw_irq_enable(0);
        int resultado_fila = queue_add(s->fila, current_task);
        hw_irq_enable(1);

        if(resultado_fila==0){

            spin_unlock(&s->lock);
            task_switch(task_kernel);

            if(current_task->exit_code == ERROR){
                current_task->exit_code = 0;
                return ERROR;
            }
            return NOERROR;
        }
        else{
            current_task->status = EXECUTING;
            spin_unlock(&s->lock);
            return ERROR;
        }
    }
    spin_unlock(&s->lock);
    return NOERROR;
}

int sem_up(struct semaphore_t *s){


    if(!sem_existe(s)){
        return ERROR;
    }

    if(!s||!s->fila){
        printf("Nao foi possivel fazer up no semaforo\n");
        return ERROR;
    }

    spin_lock(&s->lock);

    s->contador = s->contador +1;
    if(s->contador <= 0){

        hw_irq_enable(0);
        struct task_t *cabeca_fila = queue_head(s->fila);

        if(cabeca_fila != NULL){
            queue_del(s->fila,cabeca_fila);
            cabeca_fila->status = READY;
            queue_add(ready_queue,cabeca_fila);
        }
        hw_irq_enable(1);
    }
    
    spin_unlock(&s->lock);
    return NOERROR;
}
