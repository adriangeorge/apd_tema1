#include "genetic_algorithm.h"
#include "stdio.h"
#include "time.h"
#include <stdlib.h>
int main(int argc, char *argv[])
{
    struct timespec start_c, finish_c;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start_c);

    // array with all the objects that can be placed in the sack
    sack_object *objects = NULL;

    // number of objects
    int object_count = 0;

    // maximum weight that can be carried in the sack
    int sack_capacity = 0;

    // number of generations
    int generations_count = 0;

    if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, argc, argv))
    {
        return 0;
    }

    run_genetic_algorithm(objects, object_count, generations_count, sack_capacity);

    free(objects);
    clock_gettime(CLOCK_MONOTONIC, &finish_c);
    elapsed = (finish_c.tv_sec - start_c.tv_sec);
    elapsed += (finish_c.tv_nsec - start_c.tv_nsec) / 1000000000.0;
    //printf("Time : %f\n", elapsed);
    return 0;
}
