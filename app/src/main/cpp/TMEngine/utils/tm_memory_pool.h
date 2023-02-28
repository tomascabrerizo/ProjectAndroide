//
// Created by Manuel Cabrerizo on 26/02/2023.
//

#ifndef MY_APPLICATION_TM_MEMORY_POOL_H
#define MY_APPLICATION_TM_MEMORY_POOL_H

#define HEADER_SIZE sizeof(unsigned char *)

struct TMMemoryPool {
    unsigned char **blockArray;
    unsigned char *head;
    unsigned int chunkSize;
    unsigned int numChunk;
    unsigned int blockCount;
};

TMMemoryPool *TMMemoryPoolCreate(unsigned int chunkSize, unsigned int numChunk);
void TMMemoryPoolDestroy(TMMemoryPool *memoryPool);
void *TMMemoryPoolAlloc(TMMemoryPool *memoryPool);
void TMMemoryPoolFree(TMMemoryPool *memoryPool, void *mem);

#endif //MY_APPLICATION_TM_MEMORY_POOL_H
