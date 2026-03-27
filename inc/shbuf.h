#ifndef SHBUF_H
#define SHBUF_H

#include <semaphore.h>
#include <stdint.h>

/* Shared memory circular buffer for signal samples */
typedef struct {
    uint32_t capacity;      /* Maximum number of samples in the buffer */
    uint32_t write_idx;     /* Current write position (head) */

    double fs;              /* Sampling frequency in Hz */

    sem_t mutex;            /* Semaphore for synchronization */    
    double data[];          /* Flexible array for signal data */
} shbuf_t;

#endif 