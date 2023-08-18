//pagetable.h
#ifndef PAGETABLE_H
#define PAGETABLE_H

#include "header.h"

struct pageMap; 
struct ProcessPageTable; 

void insert(ProcessPageTable* table, int pageNumber, int frameNumber);
int get_frame_number(ProcessPageTable* table, int pageNumber);
int printtable(ProcessPageTable *table);

#endif