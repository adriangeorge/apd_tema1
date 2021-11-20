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
    props common_props;

    if (!read_input(&common_props, argc, argv))
    {
        return 0;
    }

    common_props.b = &barrier;
    props *genetic_props = (props *)malloc(common_props.num_threads * sizeof(props));
    // Create longer list to the next power of 2 (will be used for merge sort)
    common_props.next_pow = pow(2, ceil(log(common_props.object_count) / log(2)));
    common_props.tmp = (individual *)calloc(common_props.next_pow, sizeof(individual));
    common_props.current_generation = (individual *)calloc(common_props.next_pow, sizeof(individual));
    common_props.next_generation = (individual *)calloc(common_props.next_pow, sizeof(individual));
    common_props.vNew = (individual *)calloc(common_props.next_pow, sizeof(individual));

    common_props.g_curr_gen = &common_props.current_generation;
    common_props.g_vnew = &common_props.vNew;

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
    return 0;
}
