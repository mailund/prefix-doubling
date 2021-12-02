#ifndef SHARED_H
#define SHARED_H

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define NO_CHARS (1 << CHAR_BIT)
#define SWAP(a, b)             \
    do                         \
    {                          \
        __typeof__(a) tmp = a; \
        a = b;                 \
        b = tmp;               \
    } while (0)

void *abort_malloc(size_t size);

// Get the rank from array rank, but if you index beyond the end,
// you get the sentinel letter 0.
static inline uint32_t get_rank(uint32_t n, uint32_t const rank[n], uint32_t i)
{
    return (i < n) ? rank[i] : 0;
}

// Fills sa with indices 0, 1, 2, ..., n - 1
void fill_indices(uint32_t n, uint32_t sa[n]);

// Computes the initial rank of x, r0, from the first character in
// each suffix. Returns the number of distinct characters in x.
uint32_t calc_rank0(uint32_t n, char const x[n], uint32_t r0[n]); // FlawFinder: ignore (x[n] is fine)

void random_string(uint32_t n, char x[n + 1]);                            // FlawFinder: ignore (x[n] is fine)
void assert_sa_sorted(uint32_t n, char const x[n], uint32_t const sa[n]); // FlawFinder: ignore (x[n] is fine)

typedef void (*construction_func)(uint32_t n, char const x[n], uint32_t sa[n]);              // FlawFinder: ignore (x[n] is fine)
clock_t time_construction(construction_func f, uint32_t n, char const x[n], uint32_t sa[n]); // FlawFinder: ignore (x[n] is fine)

void performance_measurements(int no_funcs,
                              char const *func_names[no_funcs], // FlawFinder: ignore (fixed length buffer is fine here)
                              construction_func funcs[no_funcs],
                              uint32_t n, int reps);

#endif
