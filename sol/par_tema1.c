#define _XOPEN_SOURCE 600
#include "par_genetic_algorithm.h"
#include "time.h"
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
pthread_barrier_t barrier;

int main(int argc, char *argv[])
{
    struct timespec start_c, finish_c;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start_c);
    props common_props;

    if (!read_input(&common_props, argc, argv))
    {
        return 0;
    }

    common_props.b = &barrier;
    props *genetic_props = (props *)malloc(common_props.num_threads * sizeof(props));
    common_props.current_generation = (individual *)calloc(common_props.object_count, sizeof(individual));
    common_props.next_generation = (individual *)calloc(common_props.object_count, sizeof(individual));
    common_props.vNew = (individual *)calloc(common_props.object_count, sizeof(individual));
    common_props.tmp = NULL;

    for (int i = 0; i < common_props.num_threads; i++)
    {
        genetic_props[i] = common_props;
        genetic_props[i].id = i;
    }

    pthread_t *threads;
    threads = (pthread_t *)malloc(common_props.num_threads * sizeof(pthread_t));

    pthread_barrier_init(&barrier, NULL, common_props.num_threads);
    int r;
    for (int i = 0; i < common_props.num_threads; i++)
    {
        r = pthread_create(&threads[i], NULL, run_genetic_algorithm_par, &genetic_props[i]);

        if (r)
        {
            printf("Eroare la crearea thread-ului %d\n", i);
            exit(-1);
        }
    }

    void *status;
    for (int i = 0; i < common_props.num_threads; i++)
    {
        r = pthread_join(threads[i], &status);

        if (r)
        {
            printf("Eroare la asteptarea thread-ului %d\n", i);
            exit(-1);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &finish_c);
    elapsed = (finish_c.tv_sec - start_c.tv_sec);
    elapsed += (finish_c.tv_nsec - start_c.tv_nsec) / 1000000000.0;
    //printf("Time : %f\n", elapsed);
    return 0;
}
