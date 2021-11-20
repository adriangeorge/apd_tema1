#include "par_genetic_algorithm.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
int read_input(props *g_props, int argc, char *argv[])
{
    FILE *fp;

    if (argc < 3)
    {
        fprintf(stderr, "Usage:\n\t./tema1 in_file generations_count\n");
        return 0;
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL)
    {
        return 0;
    }

    if (fscanf(fp, "%d %d", &g_props->object_count, &g_props->sack_capacity) < 2)
    {
        fclose(fp);
        return 0;
    }

    if (g_props->object_count % 10)
    {
        fclose(fp);
        return 0;
    }

    sack_object *tmp_objects = (sack_object *)calloc(g_props->object_count, sizeof(sack_object));

    for (int i = 0; i < g_props->object_count; ++i)
    {
        if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2)
        {
            free(g_props->objects);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);

    g_props->generations_count = (int)strtol(argv[2], NULL, 10);

    g_props->num_threads = (int)strtol(argv[3], NULL, 10);

    if (g_props->generations_count == 0)
    {
        free(tmp_objects);

        return 0;
    }

    g_props->objects = tmp_objects;

    return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
    for (int i = 0; i < object_count; ++i)
    {
        printf("%d %d\n", objects[i].weight, objects[i].profit);
    }
}

void print_generation(const individual *generation, int limit)
{
    for (int i = 0; i < limit; ++i)
    {
        for (int j = 0; j < generation[i].chromosome_length; ++j)
        {
            printf("%d ", generation[i].chromosomes[j]);
        }

        printf("\n%d - %d\n", i, generation[i].fitness);
    }
}

void print_best_fitness(const individual *generation)
{
    printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int object_count, int sack_capacity, props *g_props)
{
    int weight;
    int profit;
    int start = g_props->id * (double)g_props->object_count / g_props->num_threads;
    int end = min((g_props->id + 1) * (double)g_props->object_count / g_props->num_threads, g_props->object_count);
    for (int i = start; i < end; ++i)
    {
        weight = 0;
        profit = 0;

        for (int j = 0; j < generation[i].chromosome_length; ++j)
        {
            if (generation[i].chromosomes[j])
            {
                weight += objects[j].weight;
                profit += objects[j].profit;
            }
        }

        generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
    }
}

int cmpfunc(const void *a, const void *b)
{
    int i;
    individual *first = (individual *)a;
    individual *second = (individual *)b;

    int res = second->fitness - first->fitness; // decreasing by fitness
    if (res == 0)
    {
        int first_count = 0, second_count = 0;

        for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i)
        {
            first_count += first->chromosomes[i];
            second_count += second->chromosomes[i];
        }

        res = first_count - second_count; // increasing by number of objects in the sack
        if (res == 0)
        {
            return second->index - first->index;
        }
    }

    return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index, props *g_props)
{
    int i, mutation_size;
    int step = 1 + generation_index % (ind->chromosome_length - 2);

    if (ind->index % 2 == 0)
    {
        // for even-indexed individuals, mutate the first 40% chromosomes by a given step
        mutation_size = ind->chromosome_length * 4 / 10;
        // offset thread ids by half (negative) of thread count
        int off_th_id = g_props->id;
        int start = off_th_id * (double)mutation_size / (g_props->num_threads / 2);
        int end = min((off_th_id + 1) * (double)mutation_size / (g_props->num_threads / 2), mutation_size);
        for (i = start; i < end; i += step)
        {
            ind->chromosomes[i] = 1 - ind->chromosomes[i];
        }
    }
    else
    {
        // for even-indexed individuals, mutate the last 80% chromosomes by a given step
        mutation_size = ind->chromosome_length * 8 / 10;
        for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step)
        {
            ind->chromosomes[i] = 1 - ind->chromosomes[i];
        }
    }
}

void mutate_bit_string_2(const individual *ind, int generation_index, props *g_props)
{
    int step = 1 + generation_index % (ind->chromosome_length - 2);

    // mutate all chromosomes by a given step
    // offset thread ids by half (negative) of thread count
    int off_th_id = g_props->id - (g_props->num_threads / 2);
    int start = off_th_id * (double)ind->chromosome_length / (g_props->num_threads / 2);
    int end = min((off_th_id + 1) * (double)ind->chromosome_length / (g_props->num_threads / 2), ind->chromosome_length);
    for (int i = start; i < end; i += step)
    {
        ind->chromosomes[i] = 1 - ind->chromosomes[i];
    }
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
    individual *parent2 = parent1 + 1;
    individual *child2 = child1 + 1;
    int count = 1 + generation_index % parent1->chromosome_length;

    memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
    memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

    memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
    memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
    memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}

void free_generation(individual *generation, props *g_props)
{
    int i;

    int start = g_props->id * (double)generation->chromosome_length / g_props->num_threads;
    int end = min((g_props->id + 1) * (double)generation->chromosome_length / g_props->num_threads, generation->chromosome_length);
    for (i = start; i < end; ++i)
    {
        free(generation[i].chromosomes);
        generation[i].chromosomes = NULL;
        generation[i].fitness = 0;
    }
}

void merge(individual *source, int start, int mid, int end, individual *destination)
{
    int iA = start;
    int iB = mid;
    int i = 0;

    for (i = start; i < end; i++)
    {
        // comparisons
        int res = source[iB].fitness - source[iA].fitness; // decreasing by fitness
        if (res == 0)
        {
            int first_count = 0, second_count = 0;

            for (int j = 0; j < source[iA].chromosome_length && j < source[iB].chromosome_length; ++j)
            {
                first_count += source[iA].chromosomes[j];
                second_count += source[iA].chromosomes[j];
            }

            res = first_count - second_count; // increasing by number of objects in the sack
            if (res == 0)
            {
                res = source[iB].index - source[iA].index;
            }
        }
        // cmpfunc((const void *)&source[iA], (const void *)&source[iB]) <= 0)
        // source[iB].fitness >= source[iA].fitness
        if (end == iB || (iA < mid && (cmpfunc((const void *)&source[iA], (const void *)&source[iB]) <= 0)))
        {
            destination[i] = source[iA];
            iA++;
        }
        else
        {
            destination[i] = source[iB];
            iB++;
        }
    }
}

void mergeSort_paralel(props *g_props, individual *current_generation, individual *vNew)
{

    int thread_id = g_props->id;
    int width = 0;
    int i = 0;

    // aux obj vector
    individual *aux = NULL;

    // implementati aici merge sort paralel
    for (width = 1; width < g_props->object_count; width *= 2)
    {
        int piese = g_props->object_count / (2 * width);

        int start = (thread_id * piese / g_props->num_threads) * 2 * width;
        int end = ((thread_id + 1) * piese) / g_props->num_threads * 2 * width;

        for (i = start; i < end; i += 2 * width)
        {
            merge(current_generation, i, i + width, i + 2 * width, vNew);
        }

        pthread_barrier_wait(g_props->b);

        if (thread_id == 0)
        {
            aux = current_generation;
            current_generation = vNew;
            vNew = aux;
        }

        pthread_barrier_wait(g_props->b);
    }
}

void *run_genetic_algorithm_par(void *genetic_props)
{
    // Assigning props
    props *g_props = (props *)genetic_props;
    int object_count = g_props->object_count;
    sack_object *objects = g_props->objects;
    int sack_capacity = g_props->sack_capacity;
    individual *current_generation = g_props->current_generation;
    individual *next_generation = g_props->next_generation;
    individual *tmp = NULL;

    // set initial generation (composed of object_count individuals with a single item in the sack)
    int count, cursor;
    int start = g_props->id * (double)g_props->object_count / g_props->num_threads;
    int end = min((g_props->id + 1) * (double)g_props->object_count / g_props->num_threads, g_props->object_count);
    for (int i = start; i < end; ++i)
    {
        current_generation[i].fitness = 0;
        current_generation[i].chromosomes = (int *)calloc(object_count, sizeof(int));
        current_generation[i].chromosomes[i] = 1;
        current_generation[i].index = i;
        current_generation[i].chromosome_length = object_count;

        next_generation[i].fitness = 0;
        next_generation[i].chromosomes = (int *)calloc(object_count, sizeof(int));
        next_generation[i].index = i;
        next_generation[i].chromosome_length = object_count;
    }

    pthread_barrier_wait(g_props->b);
    // iterate for each generation
    for (int k = 0; k < g_props->generations_count; ++k)
    {
        cursor = 0;

        compute_fitness_function(objects, current_generation, object_count, sack_capacity, g_props);
        pthread_barrier_wait(g_props->b);

        // TODO: Replace qsort with parallel merge sort
        if (g_props->id == 0)
            qsort(current_generation, object_count, sizeof(individual), cmpfunc);

        pthread_barrier_wait(g_props->b);

        // keep first 30% children (elite children selection)
        count = object_count * 3 / 10;
        start = g_props->id * (double)count / g_props->num_threads;
        end = min((g_props->id + 1) * (double)count / g_props->num_threads, count);
        for (int i = start; i < end; ++i)
        {
            copy_individual(current_generation + i, next_generation + i);
        }
        pthread_barrier_wait(g_props->b);
        cursor = count;

        // start = g_props->id * (double)count / g_props->num_threads;
        // end = min((g_props->id + 1) * (double)count / g_props->num_threads, count);

        count = object_count * 2 / 10;
        if (g_props->id < g_props->num_threads / 2)
        {
            // mutate first 20% children with the first version of bit string mutation
            for (int i = start; i < end; ++i)
            {
                copy_individual(current_generation + i, next_generation + cursor + i);
                mutate_bit_string_1(next_generation + cursor + i, k, g_props);
            }
        }
        cursor += count;

        // start = g_props->id * (double)count / g_props->num_threads;
        // end = min((g_props->id + 1) * (double)count / g_props->num_threads, count);

        count = object_count * 2 / 10;
        if (g_props->id >= g_props->num_threads / 2)
        {
            // mutate next 20% children with the second version of bit string mutation
            for (int i = start; i < end; ++i)
            {
                copy_individual(current_generation + i + count, next_generation + cursor + i);
                mutate_bit_string_2(next_generation + cursor + i, k, g_props);
            }
        }
        cursor += count;

        // crossover first 30% parents with one-point crossover
        // (if there is an odd number of parents, the last one is kept as such)
        count = object_count * 3 / 10;

        if (count % 2 == 1)
        {
            copy_individual(current_generation + object_count - 1, next_generation + cursor + count - 1);
            count--;
        }

        for (int i = 0; i < count; i += 2)
        {
            crossover(current_generation + i, next_generation + cursor + i, k);
        }

        // switch to new generation
        tmp = current_generation;
        current_generation = next_generation;
        next_generation = tmp;

        start = g_props->id * (double)object_count / g_props->num_threads;
        end = min((g_props->id + 1) * (double)object_count / g_props->num_threads, object_count);
        for (int i = start; i < end; ++i)
        {
            current_generation[i].index = i;
        }
        pthread_barrier_wait(g_props->b);
        if (k % 5 == 0 && g_props->id == 0)
        {
            print_best_fitness(current_generation);
        }
    }

    compute_fitness_function(objects, current_generation, object_count, sack_capacity, g_props);
    pthread_barrier_wait(g_props->b);
    if (g_props->id == 0)
    {
        qsort(current_generation, object_count, sizeof(individual), cmpfunc);
        print_best_fitness(current_generation);
    }

    if (g_props->id == 0)
    {
        // free resources for old generation
        free_generation(current_generation, g_props);
        free_generation(next_generation, g_props);

        // free resources
        free(current_generation);
        free(next_generation);
    }
    pthread_exit(NULL);
    return NULL;
}