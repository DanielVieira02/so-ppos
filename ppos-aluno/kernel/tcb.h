// GRR20242288 Eduardo Munaretto Majczak
// GRR20242306 João Pedro Oliveira Lazari
// GRR20206889 Daniel Henrique Vieira
// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 2.0 -- Junho de 2025

// Este arquivo PODE/DEVE ser alterado.

// Descritor de tarefas (TCB - Task Control Block).

#ifndef __PPOS_TCB__
#define __PPOS_TCB__

#include "ctx.h"

#define READY       0
#define EXECUTING   1
#define DONE        2
#define SUSPENDED   3
#define SLEEPING    4


// Task Control Block (TCB), infos sobre uma tarefa
struct task_t
{
    int id;                             // identificador da tarefa
    int vg_id;
    char *name;                         // nome da tarefa
    struct ctx_t context;               // contexto armazenado da tarefa
    int status;                         // pronta, executando, terminada, ...
    int exit_code;
    struct task_t * task_pai;
    struct task_t * task_wait;          // tarefa que esta tarefa estah esperando
    struct queue_t *suspend_queue;      // lista de tarefas esperando a atual
    int static_priority;
    int dynamic_priority;
    unsigned int alive_time;            // tempo total vivo
    unsigned int cpu_time;              // tempo total de cpu usada
    unsigned int number_activation;     // qnt de ativacoes
    unsigned int birth_time;            // guarda o momento em que nasceu
    int quantum;
    int wake_time;
                                //  ...
};

#endif
