#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <math.h>
#include <pthread.h>

#include "header.h"
// #include "memory.h"

pthread_mutex_t frame_table_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int pageNo;
    ProcessAttri pcb;
} Frame; 

extern pthread_cond_t deallocation_cond;
extern pthread_mutex_t state_mutex;

Frame frame_table[NUM_FRAMES];

void initialize_frame_table() {
    printf("\nInitializing RAM with frames...\n");
    for(int i =0; i < NUM_FRAMES; i++){
        frame_table[i].pcb.pid = -1 ;
        frame_table[i].pcb.state = -1 ;
        frame_table[i].pageNo = -1;
    }
}

void print_frametable(){
    printf("\n\n\nFrame Entry Table:\n");
    printf("--------------------\n");
    printf("| Frame |  PID  | State |\n");
    printf("--------------------\n");
    for (int i = 0; i < NUM_FRAMES; i++) {
        printf("|   %3d |   %3d |   %3d |\n", i, frame_table[i].pcb.pid, frame_table[i].pcb.state);
    }
    printf("--------------------\n");
}

int allocate_frame(int pid, int state, int pageNo){
    pthread_mutex_lock(&frame_table_mutex);
    for(int i= 0; i < NUM_FRAMES; i++){
        if(frame_table[i].pcb.pid == -1){
            frame_table[i].pcb.pid = pid;
            frame_table[i].pcb.state = state;
            frame_table[i].pageNo = pageNo;
            // if(PROCESS_COUNT*FRAME_SZ-1 == i){
            //     print_frametable();
            // }
            return i;
        }
    }
    pthread_mutex_unlock(&frame_table_mutex);
    return -1; //no frame empty 
}

void deallocate_frame(int frameNo){
    pthread_mutex_lock(&frame_table_mutex);
    printf("Deallocating Frame %d, Process %d\n", frameNo, frame_table[frameNo].pcb.pid);
    frame_table[frameNo].pcb.pid = -1;
    frame_table[frameNo].pcb.state = -1;
    frame_table[frameNo].pageNo = -1;
    pthread_mutex_unlock(&frame_table_mutex);
}


void *frame_deallocation(void *arg){

    FrameDeallocationArgs *deallocationArgs = (FrameDeallocationArgs *)arg;
    ProcessAttri *processes = deallocationArgs->processes;
            
    while (1) {
        print_frametable();           
        // pthread_mutex_lock(&state_mutex);
        // printf("FRAME DEALLOC LOCKED MUTEX\n");
        // pthread_cond_wait(&deallocation_cond, &state_mutex);
        printf("Checking for terminated Processes. \n");
        int allTerminated = 1;
        usleep(500);

        pthread_mutex_lock(&frame_table_mutex);
        for (int i = 0; i < NUM_FRAMES; i++) {
                // printf("//InsideDealloc Frame %d - Process %d state %d\n", i,  
                        // processes[frame_table[i].pcb.pid].pid, 
                        // processes[frame_table[i].pcb.pid].state);                    
                if (frame_table[i].pcb.pid != -1) {
                    if (processes[frame_table[i].pcb.pid].state == 0) {
                        deallocate_frame(i);
                        // if( i == PROCESS_COUNT*FRAME_SZ - 1){print_frametable();}                    
                    } else {
                        allTerminated = 0;
                    }
            }
        } 
        pthread_mutex_unlock(&frame_table_mutex);

        // pthread_mutex_unlock(&state_mutex); 
        // printf("FRAME DEALLOC UN-LOCKED MUTEX\n");
        if(allTerminated) {
            printf("\nAll Frames Deallocated. Exiting...\n");
            break;
        }
    }

    pthread_mutex_unlock(&frame_table_mutex);

    return NULL; 
}