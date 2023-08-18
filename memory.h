#ifndef MEMORY_H
#define MEMORY_H

#include "header.h"

struct FrameDeallocation;

// typedef struct {
//     int pid; 
//     int state;
// } ProcessAttri; 


void initialize_frame_table( void );
int allocate_frame(int pid, int state, int pageNo);
void deallocate_frame(int frameNo); 
void *frame_deallocation(void *arg);

#endif