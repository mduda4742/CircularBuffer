#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <math.h>

#include "shbuf.h"

const char* SHM_NAME = "/data_shm"; /* Name of the shared memory segment */
volatile sig_atomic_t stop = 0; /* Flag for clean shutdown */

/* Signal handler for clean shutdown */
void handle_sigint(int sig) {
    (void)sig;
    stop = 1;
}

int main(int argc, char* argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Usage: %s <f1> <fs> <DC> <buffer_size>\n", argv[0]);
        return EXIT_FAILURE;
    }

    double f1 = atof(argv[1]);
    double fs = atof(argv[2]);
    double dc = atof(argv[3]);
    int buffer_size = (uint32_t) atoi(argv[4]);

    if (f1 <= 0 || fs <= 0 || buffer_size <= 0) {
        fprintf(stderr, "Error: all arguments must be positive (except DC)\n");
        return EXIT_FAILURE;
    }

    /* Check Nyquist criterion */
    if (fs < 2 * f1) {
        fprintf(stderr, "Warning: fs is lesser than 2*f1 (aliasing)\n");
    }

    /* Set up signal handler for clean shutdown */
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* Create shared memory segment */
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR | O_TRUNC, 0666);
    if (fd == -1) {
        perror("shm_open"); 
        return EXIT_FAILURE;
    }

    /* Set size of the shared memory segment: metadata + flexible array */
    const size_t SIZE = sizeof(shbuf_t) + (buffer_size * sizeof(double));
    if (ftruncate(fd, SIZE) == -1) {
        perror("ftruncate"); 
        return EXIT_FAILURE;
    }

    /* Map shared memory */
    shbuf_t *data = (shbuf_t*) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap"); 
        return EXIT_FAILURE; 
    }

    /* Initialize semaphore */
    if (sem_init(&data->mutex, 1, 1) == -1) {
        perror("sem_init");
        munmap(data, SIZE);
        return EXIT_FAILURE;
    }

    /* Initialize buffer parameters */
    data->capacity = buffer_size;
    data->write_idx = 0;
    data->fs = fs;

    /* Real-time configuration */
    unsigned long long n = 0;
    struct timespec req;
    double period = 1.0 / fs;
    req.tv_sec = (time_t)period;
    req.tv_nsec = (long)((period - req.tv_sec) * 1000000000L);

    printf("[vSensor] Start: f1=%.1f, fs=%.1f, DC=%.1f, N=%d\n", f1, fs, dc, buffer_size);

    /* Main loop */
    while (!stop) {
        double val = dc + sin(2.0 * M_PI *  f1 * n / fs);

        /* Synchronized write to circular buffer */
        sem_wait(&data->mutex);
        data->data[data->write_idx] = val;
        data->write_idx = (data->write_idx + 1) % data->capacity    ;
        sem_post(&data->mutex);
        
        n++;
        nanosleep(&req, NULL);
    }

    /* Cleanup */
    sem_destroy(&data->mutex);
    munmap(data, SIZE);  
    shm_unlink(SHM_NAME);       
    close(fd);     

    printf("\n[vSensor] Clean shutdown.\n");
    return 0;
}