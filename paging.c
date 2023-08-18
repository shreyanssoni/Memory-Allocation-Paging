#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "header.h"
#include "readyqueue.h"
#include "memory.h"
#include "pagetable.h"

pthread_mutex_t state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t deallocation_cond = PTHREAD_COND_INITIALIZER; 

static pthread_mutex_t ready_queue_mutex = PTHREAD_MUTEX_INITIALIZER; 

void initialize_table(ProcessPageTable tables[], int processesCount){
    for(int i=0; i< processesCount; i++){
        for(int j=0; j< TABLE_SIZE; j++){
            insert(&tables[i], j,-1);
        }
    }
}

// int get_frames( ProcessPageTable *table, int pid, int state, int pages){
//     int frameNumber;
//     for(int i =0 ; i < pages; i++){
//         frameNumber = allocate_frame(pid, state, i);
//         if (frameNumber >= 0 ){
//             insert(table, i, frameNumber);
//             printf("Process %d, Page %d Mapped to --> Frame %d\n", pid, i, frameNumber);
//             return 1; 
//         } else {
//             printf("Error Mapping. %d !-> %d \n", i, frameNumber);
//             return -1; 
//         }
//     }
// }

// void *process_thread(void *arg){

//     ThreadArgs *threadArgs = (ThreadArgs*)arg;

//     PCB *process = &threadArgs->processes[threadArgs->processIndex]; 

//     // pthread_cond_signal(&deallocation_cond);
//     printf("\nProcess %d terminated with state %d...\n", process->pid, process->state);
//     return NULL;
// }

void *ready_queue_thread(void *arg) {
    // for(int i =0 ; i < PROCESS_COUNT; i++){
    //      
        // usleep(1000); 
    // }
    int gotFrame = -1; 

    ThreadArgs *threadArgs = (ThreadArgs*)arg;
    PCB *process = &threadArgs->processes[threadArgs->processIndex]; 

    pthread_mutex_lock(&state_mutex);
    // printf("PTHREAD CREATE LOCKED MUTEX\n");

    srand(time(NULL) + process->pid);
    int randomSleepTime = rand() % 2 + 1;

    sleep(randomSleepTime); 
    process->state = 1; 
    printf("State of process %d = %d\n", process->pid , process->state);  
    pthread_mutex_unlock(&state_mutex);
    // printf("PTHREAD CREATE UN-LOCKED MUTEX\n");
    pthread_cond_signal(&deallocation_cond); 
    // printf("Escaped the mutex lock in paging 64 line\n");
    usleep(100);
    // gotFrame = get_frames(&threadArgs->page_tables[threadArgs->processIndex], process->pid, process->state, threadArgs->pages);
    //     if(gotFrame){
    //         sleep(3*process->pid + 1);
    //     }

    return NULL; 
}

void print_info(PCB *processes){
    for(int i=0; i <PROCESS_COUNT; i++){
        printf("Process %d State %d\n", processes[i].pid , processes[i].state);
    }
}

void* enqueue_thread(void *arg) {
    ThreadArgs *threadArgs = (ThreadArgs*)arg;
    PCB *processes = threadArgs->processes;

    while(1){
        usleep(1000);
        for (int i = 0; i < PROCESS_COUNT; i++) {
            PCB *process = &processes[i];

            if (process->state == 1) {
                pthread_mutex_lock(&ready_queue_mutex);
                printf("Ready to QUEUE process %d , state %d", process->pid, process->state);
                enqueue_ready_queue(process);
                pthread_mutex_unlock(&ready_queue_mutex);
            }
        // usleep(1000);
        }
    }
}

void *dequeue_thread(void *arg) {
    ThreadArgs *threadArgs = (ThreadArgs*)arg;
    PCB *processes = threadArgs->processes; 

    while(1) {
        pthread_mutex_lock(&ready_queue_mutex);
        PCB *dequeued_process = dequeue_ready_queue(threadArgs->page_tables);
        pthread_mutex_unlock(&ready_queue_mutex);
        
        if(dequeued_process == NULL ){
            usleep(4000);
            pthread_cond_signal(&deallocation_cond);
        } else {
            pthread_mutex_lock(&state_mutex);
            dequeued_process->state = 0; 
            pthread_mutex_unlock(&state_mutex);
            printf("Dequeued process %d from ready queue.\n", dequeued_process->pid);
            // usleep(500);
        }
        pthread_cond_signal(&deallocation_cond);
        usleep(10000);
    }
}

int main(){

    int pages = PROCESS_SZ / PAGE_SZ;       
    int frameSize = PAGE_SZ, frames = RAM_SZ/frameSize; 

    int pageNo = ceil(log2(pages));
    int pageOffset = PAGE_SZ - pageNo; 

    int processesCount = PROCESS_COUNT;
    ProcessPageTable page_tables[processesCount];

    initialize_frame_table(); 
    initialize_table(page_tables, processesCount);

    PCB processes[processesCount]; 

    ThreadArgs threadArgsArray[processesCount];
    pthread_t process_threads[processesCount];
    ThreadArgs threadArgs; 
    threadArgs.processes = processes; 
    threadArgs.page_tables = page_tables; 
    threadArgs.processIndex = 0; 
    threadArgs.pages = pages;
    
    pthread_t deallocation_thread; 
    pthread_t ready_queue, dequeueThread, enqueueThread; 
    // pthread_create(&ready_queue, NULL, scheduler, &threadArgs); 
    pthread_create(&dequeueThread, NULL, dequeue_thread, &threadArgs);
    pthread_create(&enqueueThread, NULL, enqueue_thread, &threadArgs);

    for (int i = 0; i < processesCount; i++) {
        processes[i].pid = i;
        processes[i].state = 2;
        printf("Created Process %d with PID %d\n", i, processes[i].pid);
        usleep(100);
    }

    print_info(processes) ;
    
    for (int i = 0; i < processesCount; i++) {
        // Create a separate thread for each process
        threadArgsArray[i].processes = processes; 
        threadArgsArray[i].page_tables = page_tables; 
        threadArgsArray[i].processIndex = i; 
        threadArgsArray[i].pages = pages; 
         
        pthread_create(&process_threads[i], NULL, ready_queue_thread, &threadArgsArray[i]);        
        usleep(1000);
    } 

    struct FrameDeallocation deallocArgs;
    deallocArgs.processes = processes; 
    pthread_create(&deallocation_thread, NULL, frame_deallocation, &deallocArgs);
    
    for(int i =0; i  < processesCount; i++){
        pthread_join(process_threads[i], NULL); 
    }


    pthread_cond_signal(&deallocation_cond); 
    pthread_join(ready_queue, NULL); 
    pthread_join(dequeueThread, NULL); 
    pthread_join(enqueueThread, NULL); 

    pthread_join(deallocation_thread, NULL);
    return 0; 
}