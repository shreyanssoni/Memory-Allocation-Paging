#ifndef READY_QUEUE_H
#define READY_QUEUE_H

#include "header.h"
#include <pthread.h> 

struct ThreadArgs; 

// typedef struct queuePCB{
//     int pid; 
//     int state; 
//     struct queuePCB *next; 
//     struct queuePCB *prev; 
// } queuePCB;

// typedef struct{
//     int pageNo;
//     int frameNo;
// } pageMap; 

// typedef struct {
//     pageMap pTable[TABLE_SIZE];
// } ProcessPageTable; 

struct PCB; 

void *scheduler(void *arg);

void enqueue_ready_queue(PCB* process);

PCB *dequeue_ready_queue(); 

#endif