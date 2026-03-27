#ifndef SHBUF_H
#define SHBUF_H

#include <semaphore.h>

typedef struct {
    int size;
    int free_position;
    double fs;
    sem_t sem;
    double buf[];
} shbuf_t;

#endif 