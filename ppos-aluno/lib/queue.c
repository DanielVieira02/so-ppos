// GRR20242288 Eduardo Munaretto Majczak
// GRR20242306 João Pedro Oliveira Lazari
// GRR20206889 Daniel Henrique Vieira
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.0 -- Junho de 2025

// Implementação do TAD fila genérica
#include <stdlib.h>
#include <stdio.h>
#include "queue.h"
#include "../kernel/memory.h"

struct queue_t {
    int size;
    struct node_t *itr;
    struct node_t *first;
};

struct node_t {
    void *item;
    struct node_t *next;
};

struct queue_t *queue_create() {
    struct queue_t *queue = mem_alloc(sizeof(struct queue_t));
    
    if (queue == NULL) {
        return NULL;
    }

    queue->size = 0;
    queue->first = NULL;

    return queue;
}

int queue_destroy(struct queue_t *queue) {
    if (queue == NULL) {
        return ERROR;
    }

    struct node_t *current_node = queue->first, *aux;

    while(current_node != NULL) {
        aux = current_node->next;
        mem_free(current_node);
        current_node = aux;
    }

    mem_free(queue);

    return NOERROR;
}

int queue_add(struct queue_t *queue, void *item) {
    if (queue == NULL || item == NULL) {
        return ERROR;
    }

    struct node_t *current_node = queue->first, *last_node = NULL;

    while(current_node != NULL) {
        last_node = current_node;
        current_node = last_node->next;
    }

    current_node = mem_alloc(sizeof(struct node_t));
    if (current_node == NULL) {
        return ERROR;
    }
    current_node->item = item;
    current_node->next = NULL;
    queue->size++;

    if (queue->first == NULL) {
        queue->first = current_node;
        queue_head(queue);
    } else {
        last_node->next = current_node;
    }

    return NOERROR;
}

int queue_del(struct queue_t *queue, void *item) {
    if (queue == NULL || item == NULL || queue->size == 0) {
        return ERROR;
    }

    struct node_t *current_node, *last_node;

    current_node = queue->first;
    last_node = queue->first;

    if (current_node->item == item) {
        queue->first = current_node->next;
    } else {
        while(current_node != NULL && current_node->item != item) {
            last_node = current_node;
            current_node = last_node->next;
        }

        if (current_node == NULL) {
            return ERROR;
        }
    }
    
    if (queue->itr == current_node) {
        queue_next(queue);
    }

    last_node->next = current_node->next;
    mem_free(current_node);
    queue->size--;

    return NOERROR;
}

bool queue_has(struct queue_t *queue, void *item) {
    struct node_t *current_node = queue->first;

    while(current_node != NULL) {
        if (current_node->item == item) {
            return true;
        }
        current_node = current_node->next;
    }

    return false;
}

int queue_size(struct queue_t *queue) {
    if (queue == NULL) {
        return ERROR;
    }
    return queue->size;
}

void *queue_head(struct queue_t *queue) {
    if (queue == NULL || queue->size == 0) {
        return NULL;
    }
    queue->itr = queue->first;
    return queue->itr->item;
}

void *queue_next(struct queue_t *queue) {
    if (queue == NULL || queue->size == 0 || queue->itr == NULL) {
        return NULL;
    }

    queue->itr = queue->itr->next;

    if (queue->itr == NULL) {
        return NULL;
    }

    return queue->itr->item;
}

void *queue_item(struct queue_t *queue) {
    if (queue == NULL || queue->size == 0 || queue->itr == NULL) {
        return NULL;
    }

    return queue->itr->item;
}

void queue_print(char *name, struct queue_t *queue, void(func)(void *)) {
    printf("%s: ", name);

    if (queue == NULL) {
        printf("undef\n");
        return;
    }

    printf("[ ");
    struct node_t *current_node = queue->first;

    while(current_node != NULL) {
        if (func == NULL) {
            printf("undef ");
        } else {
            func(current_node->item);
            printf(" ");
        }
        current_node = current_node->next;
    }

    printf("] (%d items)\n", queue->size);
}