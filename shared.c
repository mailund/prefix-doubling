#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "shared.h"

void *abort_malloc(size_t size)
{
    void *mem = malloc(size);
    if (!mem)
    {
        fprintf(stderr, "Could not allocate memory.\n");
        abort();
    }

    return mem;
}

void fill_indices(uint32_t n, uint32_t sa[n])
{
    for (uint32_t i = 0; i < n; i++)
        sa[i] = i;
}

uint32_t calc_rank0(uint32_t n, char const x[n], uint32_t r0[n]) // FlawFinder: ignore (x[n] is fine)
{
    uint32_t alphabet[NO_CHARS] = {0};

    // run through x and tag occurring letters
    for (uint32_t i = 0; i < n; i++)
        alphabet[(unsigned char)x[i]] = 1;

    // assign numbers to each occurring letter
    uint32_t sigma = 1;
    for (int a = 0; a < NO_CHARS; a++)
    {
        if (alphabet[a])
            alphabet[a] = sigma++;
    }

    // map each letter from x to its number and place them in mapped
    for (uint32_t i = 0; i < n; i++)
        r0[i] = alphabet[(unsigned char)x[i]];

    return sigma;
}

void random_string(uint32_t n, char x[n + 1]) // FlawFinder: ignore (x[n] is fine)
{
    static char alphabet[] = "abcdefg";
    static char sigma = sizeof alphabet - 1; // we don't want '\0'
    for (uint32_t i = 0; i < n; i++)
        x[i] = alphabet[rand() % sigma];
    x[n] = '\0';
}

void assert_sa_sorted(uint32_t n, char const x[n], uint32_t const sa[n]) // FlawFinder: ignore (x[n] is fine)
{
    for (uint32_t i = 1; i < n; i++)
    {
        if (strcmp(x + sa[i - 1], x + sa[i]) >= 0)
        {
            fprintf(stderr, "sa[%u] = %u : %s\n", i - 1, sa[i - 1], x + sa[i - 1]);
            fprintf(stderr, "sa[%u] = %u : %s\n", i, sa[i], x + sa[i]);
            abort();
        }
    }
}

clock_t time_construction(construction_func f, uint32_t n, char const x[n], uint32_t sa[n]) // FlawFinder: ignore (x[n] is fine)
{
    // Time suffix array construction.
    clock_t then = clock();
    f(n, x, sa);
    clock_t now = clock();

    // make sure that the function worked!
    assert_sa_sorted(n, x, sa);

    return now - then;
}

void performance_measurements(int no_funcs,
                              char const *func_names[no_funcs], // FlawFinder: ignore (fixed length is fine)
                              construction_func funcs[no_funcs],
                              uint32_t n, int reps)
{
    char *x = abort_malloc(n + 1);
    uint32_t *sa = abort_malloc(n * sizeof *sa);

    // output header
    printf("rep");
    for (int i = 0; i < no_funcs; i++)
    {
        printf("\t%s", func_names[i]);
    }
    printf("\n");

    for (int rep = 1; rep <= reps; rep++)
    {
        random_string(n, x);
        printf("%d", rep);

        clock_t first = time_construction(funcs[0], n, x, sa);
        printf("\t%lu", first);
        for (int i = 1; i < no_funcs; i++)
        {
            clock_t t = time_construction(funcs[i], n, x, sa);
            printf("\t%lu (%.0f%%)", t, 100.0 * t / first);
        }
        printf("\n");
    }

    free(x);
    free(sa);
}
