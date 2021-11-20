#define _XOPEN_SOURCE 600
#ifndef GENETIC_ALGORITHM_H
#define GENETIC_ALGORITHM_H

#include "../skel/individual.h"
#include "../skel/sack_object.h"

#include <pthread.h>

// thread props structure
typedef struct _props
{
    // id of threads
    int id;

    // number of threads
    int num_threads;

    // array with all the objects that can be placed in the sack
    sack_object *objects;

    // number of objects
    int object_count;

    // maximum weight that can be carried in the sack
    int sack_capacity;

    // number of generations
    int generations_count;

    pthread_barrier_t *b;

    individual *current_generation;
    individual *next_generation;
    individual *vNew;
    individual *tmp;
} props;

// reads input from a given file
int read_input(props *g_props, int argc, char *argv[]);

// displays all the objects that can be placed in the sack
void print_objects(const sack_object *objects, int object_count);

// displays all or a part of the individuals in a generation
void print_generation(const individual *generation, int limit);

// displays the individual with the best fitness in a generation
void print_best_fitness(const individual *generation);

// computes the fitness function for each individual in a generation
void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity, props *g_props);

// compares two individuals by fitness and then number of objects in the sack (to be used with qsort)
int cmpfunc(const void *a, const void *b);

// performs a variant of bit string mutation
void mutate_bit_string_1(const individual *ind, int generation_index, props *g_props);

// performs a different variant of bit string mutation
void mutate_bit_string_2(const individual *ind, int generation_index, props *g_props);

// performs one-point crossover
void crossover(individual *parent1, individual *child1, int generation_index);

// copies one individual
void copy_individual(const individual *from, const individual *to);

// deallocates a generation
void free_generation(individual *generation, props *g_props);

// runs the genetic algorithm
void *run_genetic_algorithm_par(void *props);
#endif