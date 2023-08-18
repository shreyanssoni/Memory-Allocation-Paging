#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "header.h"

int hash(int key){  
    return key % TABLE_SIZE;
}

void insert(ProcessPageTable* table, int pageNumber, int frameNumber){

    int index = hash(pageNumber);
    // printf("Inserting Page %d to Frame %d\n", pageNumber, frameNumber );
    table->pTable[index].pageNo = pageNumber;
    table->pTable[index].frameNo = frameNumber;
}

int get_frame_number(ProcessPageTable* table, int pageNumber){
    int index = hash(pageNumber);

    if(table->pTable[index].pageNo == pageNumber){
        return table->pTable[index].frameNo; 
    }

    return -1;
}

int printtable(ProcessPageTable *table){
    printf("Logical Page  | Physical Frame \n");

    for(int i =0; i< TABLE_SIZE; i++){
        printf(" %d  --->  %d\n", table->pTable[i].pageNo, table->pTable[i].frameNo);
    }
    return 0; 
}



