#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <time.h>

#include "shbuf.h"

const char* SHM_NAME = "/data_shm";

int main(int argc, char* argv[]) {
    
    if (argc != 3) {
        fprintf(stderr, "Użycie: %s <nf> <np>\n", argv[0]);
        return EXIT_FAILURE;    
    }

    double nf = atof(argv[1]);
    int np = atoi(argv[2]);

    if (nf <= 0 || np <= 0) {
        fprintf(stderr, "Błąd: wszystkie argumenty muszą być dodatnie.\n");
        return EXIT_FAILURE;
    }

    int fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (fd == -1) { perror("shm_open"); return EXIT_FAILURE; }

    struct stat s;
    if (fstat(fd, &s) == -1) { perror("fstat"); return EXIT_FAILURE; }

    const size_t SIZE = s.st_size;

    volatile shbuf_t *data = (shbuf_t*) mmap(0, SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) { perror("mmap"); return EXIT_FAILURE; }

    if (data->size == 0) {
        fprintf(stderr, "Błąd: Bufor nie został jeszcze zainicjalizowany przez vSensor.\n");
        return EXIT_FAILURE;
    }

    if (data->size < np) {
        fprintf(stderr, "Błąd: ilość próbek <np> większa niż bufor\n");
        return EXIT_FAILURE;
    }

    struct timespec req;
    double period = 1.0 / nf; 
    req.tv_sec = (time_t)period; 
    req.tv_nsec = (long)((period - req.tv_sec) * 1000000000L);

    printf("[Monitor] Odświeżanie co: %ld s i %ld ns (%.2f Hz)\n", 
            req.tv_sec, req.tv_nsec, nf);

    while(1) {
        int crossed = 0;

        int current_pos = data->free_position;

        for (int i = 0; i < np; i++) {
            int curr_idx = (current_pos - 1 - i + data->size) % data->size;
            int prev_idx = (current_pos - 2 - i + data->size) % data->size;

            double curr_val = data->buf[curr_idx];
            double prev_val = data->buf[prev_idx];

            if ((prev_val <= 0 && curr_val > 0) || (prev_val >= 0 && curr_val < 0)) {
                crossed++;
            }
        }

        double estimated_f = (crossed * data->fs) / (2.0 * np);

        printf("[monitor_f] estim. f.: %.1f\n", estimated_f);

        nanosleep(&req, NULL);
    }

    return 0;
}