#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <pthread.h>

#include "header.h"
#include "memory.h"
#include "pagetable.h"

static pthread_mutex_t ready_queue_mutex = PTHREAD_MUTEX_INITIALIZER; 
extern pthread_cond_t deallocation_cond;

PCB *ready_queue_head = NULL; 

void enqueue_ready_queue(PCB *process){
    pthread_mutex_lock(&ready_queue_mutex);
    // printf("ENQUEUE TAKES THE LOCK for PROCESS %d\n", process->pid);
    
    process->next = NULL; 
    process->prev = NULL;


    if(ready_queue_head == NULL){
        ready_queue_head = process;
        // printf("Made the process %d as the head of ready queue.\n", process->pid);
    } 
    
    else {
        PCB *current = ready_queue_head;
        while (current->next != NULL){
            current = current->next;
        }

        printf("Enqueueing process %d to the ready queue.\n", process->pid);
        current->next = process; 
        process->prev = current; 

    }
    pthread_mutex_unlock(&ready_queue_mutex);
    // printf("ENQUEUE RELEASES THE LOCK %d\n", process->pid);
}


PCB *dequeue_ready_queue(ProcessPageTable tables[]) {
    int frameAvailable = -1;
    // //printf("before dequeue mutex\n"); 
    pthread_mutex_lock(&ready_queue_mutex);
    // printf("DEQUEUE TAKES THE LOCK. \n"); 

    if (ready_queue_head == NULL) {
        pthread_mutex_unlock(&ready_queue_mutex);
        // printf("Ready queue is empty.\n");
        return NULL; // Queue is empty
    }

    PCB *process = ready_queue_head;

    for(int i =0 ; i < FRAME_SZ; i++){
        int frameNumber = allocate_frame(process->pid, process->state, i);
        //printf("Frame %d, %d", frameNumber, i);
        if(frameNumber >= 0){
            frameAvailable = 1 ;
            insert(&tables[process->pid], i, frameNumber);
            //printf("Process %d, Page %d Mapped to Frame %d\n", process->pid, i, frameNumber);
        } else {
            frameAvailable = -1;
            pthread_mutex_unlock(&ready_queue_mutex);
            printf("DEQUEUE RELEASES LOCK DUE TO ERROR MAPPING\n");
            return NULL; 
        }
    }

    ready_queue_head = ready_queue_head->next;
    if(ready_queue_head != NULL){
        ready_queue_head->prev = NULL; 
    } else {
        ready_queue_head = NULL; 
    }

    process->next = NULL;
    process->prev = NULL; 
    
    pthread_mutex_unlock(&ready_queue_mutex);
    return process;
}

void* scheduler(void *arg){
    ThreadArgs *threadArgs = (ThreadArgs*) arg;
    PCB *processes = threadArgs->processes; 
    
    while(1) {

        // pthread_mutex_lock(&ready_queue_mutex);

        for(int i = 0; i < PROCESS_COUNT; i++){
            PCB *process = &processes[i];
            usleep(200);
            if(process->state == 1){
                // pthread_mutex_unlock(&ready_queue_mutex);
                enqueue_ready_queue(process); 
            }
        }
        // pthread_mutex_unlock(&ready_queue_mutex);

        PCB *dequeued_process = dequeue_ready_queue(threadArgs->page_tables);
        if(dequeued_process == NULL ){
            usleep(4000);
            pthread_cond_signal(&deallocation_cond);
        } else {
            dequeued_process->state = 0; 
            printf("Scheduler dequeued process %d from ready queue with state %d.\n", dequeued_process->pid, dequeued_process->state);
            pthread_cond_signal(&deallocation_cond);
            usleep(500);
        }
        usleep(100);
    }
    // return NULL; 
}
