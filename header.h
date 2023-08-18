#ifndef HEADER_H
#define HEADER_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>

// typedef struct PCB PCB; // Forward declaration
// typedef struct ThreadArgs ThreadArgs; // Forward declaration

// #include "pagetable.h"
// #include "memory.h"
// #include "readyqueue.h"

#define PROCESS_SZ 64
#define PAGE_SZ 8
#define RAM_SZ 2048
#define MEMORY_SZ 2048
#define FRAME_SZ 8
#define NUM_FRAMES 256
#define PROCESS_COUNT 35
#define TABLE_SIZE 10

typedef struct PCB {
    int pid;
    int state;
    struct PCB *next; 
    struct PCB *prev;
} PCB; 

extern pthread_cond_t deallocation_cond;
extern pthread_mutex_t state_mutex;


typedef struct {
    int pageNo;
    int frameNo;
} pageMap;

typedef struct {
    pageMap pTable[TABLE_SIZE];
} ProcessPageTable;

typedef struct {
    PCB *processes;
    ProcessPageTable *page_tables;
    int processIndex;
    int pages; 
} ThreadArgs; 

typedef struct FrameDeallocation{
    PCB *processes; 
} FrameDeallocation;

typedef struct {
    int pid; 
    int state;
} ProcessAttri; 

typedef struct {
    ProcessAttri *processes;
} FrameDeallocationArgs;

// typedef struct {
//     int pageNo;
//     ProcessAttri pcb;
// } Frame;

// typedef struct queuePCB{
//     int pid; 
//     int state; 
//     struct queuePCB *next; 
//     struct queuePCB *prev; 
// } queuePCB;

#endif