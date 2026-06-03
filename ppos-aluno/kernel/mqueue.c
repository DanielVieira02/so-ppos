// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência de filas de mensagens

#include <string.h>
#include <stdlib.h>
#include "../lib/queue.h"
#include "../hardware/cpu.h"
#include "semaphore.h"

struct mqueue_t {
	unsigned int n_msgs;
	unsigned int max;
	unsigned long int size;
	struct queue_t * msg_queue;
	struct semaphore_t * sem_access; // semaforo de acesso a fila
	struct semaphore_t * sem_recv; // semaforo para controlar envio de mensagens
	struct semaphore_t * sem_send; // idem para recebimento de mensagens 
};

void mqueue_init() {
}

struct mqueue_t *mqueue_create(int max_msgs, int msg_size) {
	if (max_msgs <= 0 || msg_size <= 0) return NULL;
	
	struct mqueue_t * new_queue = malloc(sizeof(struct mqueue_t));
	if (!new_queue) return NULL;
	
	// criacao e verificacao base da fila
	new_queue->n_msgs = 0;
	new_queue->max = max_msgs;
	new_queue->size = msg_size;
	new_queue->msg_queue = queue_create();
	if (!new_queue->msg_queue){
		return NULL;
	}
	
	// criacao e verificacao dos semaforos
	new_queue->sem_access = sem_create(1); // "mutex"
	if (!new_queue->sem_access){
		return NULL;
	}
	new_queue->sem_send = sem_create(max_msgs);
	if (!new_queue->sem_send){
		return NULL;
	}
	new_queue->sem_recv = sem_create(0);
	if (!new_queue->sem_recv){
		return NULL;
	}
	
	return new_queue;
};

int mqueue_destroy(struct mqueue_t *queue) {
	if (!queue) return -1;
	
	void * item = queue_head(queue->msg_queue);
	while (queue_size(queue->msg_queue) > 0) {
		if (item != NULL) {
			free(item);
		}
		queue_del(queue->msg_queue, item);
		item = queue_head(queue->msg_queue);
	}
	if (queue_destroy(queue->msg_queue) == -1) return -1;
	queue->msg_queue = NULL;
	if (sem_destroy(queue->sem_access) == -1) return -1;
	queue->sem_access = NULL;
	if (sem_destroy(queue->sem_send) == -1) return -1;
	queue->sem_send = NULL;
	if (sem_destroy(queue->sem_recv) == -1) return -1;
	queue->sem_recv = NULL;
	free(queue);
	
	return 0;
};

int mqueue_send(struct mqueue_t *queue, void *msg) {
	if (!queue || !msg) return -1;
	
	int status = sem_down(queue->sem_send); // diminui espaco para mandar mensagens
	if (status) return -1;
	status = sem_down(queue->sem_access);
	if (status) return -1;
	
	void * msg_ptr = malloc(queue->size);
	memcpy(msg_ptr, msg, queue->size); // copia mensagem, para nao depender da mensagem original ser mantida viva
	queue_add(queue->msg_queue, msg_ptr);
	queue->n_msgs++;
	
	sem_up(queue->sem_recv); // aumenta quantidade de mensagens a receber
	sem_up(queue->sem_access);

	return 0;
};

int mqueue_recv(struct mqueue_t *queue, void *msg) {
	if (!queue || !msg) return -1;

	int status = sem_down(queue->sem_recv); // diminui quantidade de mensagens a receber
	if (status) return -1;
	status = sem_down(queue->sem_access);
	if (status) return -1;
	
	void * queue_msg = queue_head(queue->msg_queue);
	memcpy(msg, queue_msg, queue->size);
	queue_del(queue->msg_queue, queue_msg);
	queue->n_msgs--;
	
	sem_up(queue->sem_send);
	sem_up(queue->sem_access); // aumenta espaco para mandar mensagens
	free(queue_msg);
	
	return 0;
};

int mqueue_msgs(struct mqueue_t *queue){
	if (!queue)	return ERROR;
	
	return queue->n_msgs;
};






