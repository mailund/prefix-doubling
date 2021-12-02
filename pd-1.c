#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pd-1.h"
#include "shared.h"

// Bucket sort the elements sa according to the rank[sa[i]+k]
// (with padded zero sentinels). The result goes to out.
static void bsort(uint32_t n,
                  uint32_t const sa[n],
                  uint32_t const rank[n],
                  uint32_t out[n],
                  uint32_t k,
                  uint32_t buckets[n],
                  uint32_t sigma)
{
    memset(buckets, 0, sigma * sizeof *buckets);

    for (uint32_t i = 0; i < n; i++)
    {
        buckets[get_rank(n, rank, sa[i] + k)]++;
    }
    for (uint32_t acc = 0, i = 0; i < sigma; i++)
    {
        uint32_t b = buckets[i];
        buckets[i] = acc;
        acc += b;
    }
    for (uint32_t i = 0; i < n; i++)
    {
        uint32_t j = sa[i];
        uint32_t r = get_rank(n, rank, j + k);
        out[buckets[r]++] = j;
    }
}

// For each element in sa, assumed sorted according to
// rank[sa[i]],rank[sa[i]+k], work out what rank
// (order of rank[sa[i]],rank[sa[i]+k]) each element has
// and put the result in out.
static uint32_t update_rank(uint32_t n,
                            uint32_t const sa[n],
                            uint32_t rank[n],
                            uint32_t out[n],
                            uint32_t k)
{
#define PAIR(i, k) (((uint64_t)rank[sa[i]] << 32) | \
                    (uint64_t)get_rank(n, rank, sa[i] + k))

    uint32_t sigma = 1; // leave zero for sentinel
    out[sa[0]] = sigma;

    uint64_t prev_pair = PAIR(0, k);
    for (uint32_t i = 1; i < n; i++)
    {
        uint64_t cur_pair = PAIR(i, k);
        sigma += (prev_pair != cur_pair);
        prev_pair = cur_pair;
        out[sa[i]] = sigma;
    }

    return sigma + 1;

#undef PAIR
}

// Construct a suffix array using prefix-doubling
void pd1(uint32_t n, char const x[n], uint32_t sa[n]) // FlawFinder: ignore (x[n] is fine)
{
    uint32_t *buckets = abort_malloc(n * sizeof *buckets);
    uint32_t *buf = abort_malloc(n * sizeof *buf);
    uint32_t *rank = abort_malloc(n * sizeof *rank);

    uint32_t sigma = calc_rank0(n, x, rank);
    fill_indices(n, sa);

    for (uint32_t k = 1; sigma < n + 1; k *= 2)
    {
        bsort(n, sa, rank, buf, k, buckets, sigma); // result in buf
        bsort(n, buf, rank, sa, 0, buckets, sigma); // result back in sa
        sigma = update_rank(n, sa, rank, buf, k);   // new rank in buf
        SWAP(rank, buf);                            // now new rank is back in rank
    }

    free(buckets);
    free(buf);
    free(rank);
}
