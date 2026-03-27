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

const char* SHM_NAME = "/data_shm";
volatile int flag = 0;

void handle_sigint(int sig) {
    flag = 1;
}

int main(int argc, char* argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Użycie: %s <f1> <fs> <buffer_size>\n", argv[0]);
        return EXIT_FAILURE;
    }

    double f1 = atof(argv[1]);
    double fs = atof(argv[2]);
    int buffer_size = atoi(argv[3]);

    if (f1 <= 0 || fs <= 0 || buffer_size <= 0) {
        fprintf(stderr, "Błąd: wszystkie argumenty muszą być dodatnie\n");
        return EXIT_FAILURE;
    }

    if (fs < 2 * f1) {
        fprintf(stderr, "Ostrzeżenie: fs jest mniejsze niż 2*f1 (aliasing)\n");
    }

    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    const size_t SIZE = sizeof(shbuf_t) + (buffer_size * sizeof(double));

    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    if (ftruncate(fd, SIZE) == -1) { perror("ftruncate"); return 1;}

    shbuf_t *data = (shbuf_t*) mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); return 1; }

    if (sem_init(&data->sem, 1, 1) == -1) {
        perror("sem_init");
        munmap(data, sizeof(shbuf_t));
        return 1;
    }

    data->size = buffer_size;
    data->free_position = 0;
    data->fs = fs;

    double val;
    unsigned long long n = 0;

    struct timespec req;
    double period = 1.0 / fs;
    req.tv_sec = (time_t)period;
    req.tv_nsec = (long)((period - req.tv_sec) * 1000000000L);

    printf("[vSensor] Start: f1=%.1f, fs=%.1f, N=%d\n", f1, fs, buffer_size);
    printf("%d\n", sizeof(shbuf_t));
    printf("%d\n", sizeof(sem_t));

    double a[];
    printf("%ld\n", sizeof(a));
    while (flag == 0) {
        sem_wait(&data->sem);
        val = sin(2.0 * M_PI *  f1 * n / fs);
        data->buf[data->free_position] = val;
        data->free_position = (data->free_position + 1) % data->size;
        n++;
        sem_post(&data->sem);

        nanosleep(&req, NULL);
    }

    shm_unlink(SHM_NAME);
    printf("\nPamięć współdzielona usunięta. Koniec programu.\n");
    return 0;
}