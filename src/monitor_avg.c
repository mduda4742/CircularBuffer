#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>
#include <signal.h>

#include "shbuf.h"

const char* SHM_NAME = "/data_shm"; /* Name of the shared memory segment */
volatile sig_atomic_t stop = 0; /* Flag for clean shutdown */

/* Signal handler for clean shutdown */
void handle_sigint(int sig) {
    (void)sig;
    stop = 1;
}


int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <nf> <np>\n", argv[0]);
        return EXIT_FAILURE;    
    }

    double nf = atof(argv[1]);
    int np = atoi(argv[2]);

    if (nf <= 0 || np <= 1) {
        fprintf(stderr, "Error: all arguments must be positive\n");
        return EXIT_FAILURE;
    }

    /* Set up signal handler for clean shutdown */
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* Open and map shared memory */
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open"); 
        return EXIT_FAILURE; 
    }

    struct stat s;
    if (fstat(fd, &s) == -1) {
        perror("fstat"); 
        return EXIT_FAILURE; 
    }
    const size_t SIZE = s.st_size;

    shbuf_t *data = (shbuf_t*) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap"); 
        return EXIT_FAILURE; 
    }

    /* Local buffer for analysis to minimize lock time */
    double *local_samples = malloc(np * sizeof(double));
    if (!local_samples) {
        perror("malloc"); 
        return EXIT_FAILURE; 
    }

    if (data->capacity == 0) {
        fprintf(stderr, "Error: Buffer not yet initialized by vSensor.\n");
        return EXIT_FAILURE;
    }

    if (data->capacity < (uint32_t) np) {
        fprintf(stderr, "Error: number of samples <np> greater than buffer\n");
        return EXIT_FAILURE;
    }

   /* Real-time configuration */
    struct timespec req;
    double period = 1.0 / nf; 
    req.tv_sec = (time_t)period; 
    req.tv_nsec = (long)((period - req.tv_sec) * 1000000000L);

    printf("[Monitor] Analyzing %d samples at %.2f Hz refresh rate\n", np, nf);

    while(!stop) {
        double sum = 0.0;

        /* Safe Data Capture (Critical Section) */
        sem_wait(&data->mutex);
        int current_pos = data->write_idx;

        for (int i = 0; i < np; i++) {
            int idx = (current_pos - 1 - i + data->capacity) % data->capacity;
            local_samples[i] = data->data[idx];
        }

        sem_post(&data->mutex);

        /* Processing */
        for (int i = 0; i < np; i++) {
            sum += local_samples[i];
        }

        double sma = sum / np;

        printf("[monitor_avg] SMA: %.3f\n", sma);

        nanosleep(&req, NULL);
    }

    /* Cleanup */
    free(local_samples);
    munmap(data, SIZE);
    close(fd);

    printf("\n[Monitor] Clean shutdown.\n");
    return 0;
}