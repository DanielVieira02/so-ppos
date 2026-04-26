// PingPongOS - PingPong Operating System

// Este arquivo PODE/DEVE ser alterado.

// Gerência básica do tempo.

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <stdlib.h>
#include "tcb.h"
#include "time.h"
#include "../hardware/cpu.h"
#include "dispatcher.h"


volatile int local_time;
struct sigaction sa;
struct itimerval timer;
extern struct task_t *current_task;
extern struct task_t *task_kernel;

int systime() {
    return local_time;
}

void timer_handler(int sig) {
    local_time++;
    
    if (current_task && current_task->id > 1) {
        current_task->quantum--;
        if (current_task->quantum <= 0) {
            current_task->quantum = QUANTUM;
            task_kernel->number_activation++;
            task_yield(); 
        }
    }
}


void time_init() {
    local_time = 0;

    hw_irq_handle(1, timer_handler); 
    hw_timer(1, 1);
}