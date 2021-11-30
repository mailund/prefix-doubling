#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pd-2.h"
#include "shared.h"

// Sort the elements sa according to the rank[sa[i]+k]
// (with padded zero sentinels) using a radix sort over
// 8-bit sub-integers. The result is left in sa; buf
// is a scratch buffer.
static void radix_sort(uint32_t n,
                       uint32_t sa[n],
                       uint32_t rank[n],
                       uint32_t buf[n],
                       uint32_t k)
{
    static uint32_t buckets[256]; // for sorting bytes

    // Radix sort over the four bytes in uint32_t. We flip buffers along the way,
    // but since we do an even number of bucket sorts we end up with the result
    // back in sa.
    for (uint32_t shift = 0; shift < 32; shift += 8)
    {
        memset(buckets, 0, sizeof buckets);

        // compute bucket offsets
        for (uint32_t i = 0; i < n; i++)
        {
            uint32_t j = sa[i];
            uint8_t b = get_rank(n, rank, j + k) >> shift;
            buckets[b]++;
        }
        for (uint32_t acc = 0, i = 0; i < 256; i++)
        {
            uint32_t b = buckets[i];
            buckets[i] = acc;
            acc += b;
        }
        // then place sa[i] in buckets
        for (uint32_t i = 0; i < n; i++)
        {
            uint32_t j = sa[i];
            uint8_t b = get_rank(n, rank, j + k) >> shift;
            buf[buckets[b]++] = j;
        }

        SWAP(sa, buf); // flip sa and buf for next iteration...
    }
}

// Sort sa according to pairs (rank[sa[i]],rank[sa[i]+k]).
// The result is left in sa; buf is a scratch buffer.
static void sort_pairs(uint32_t n,
                       uint32_t sa[n],
                       uint32_t rank[n],
                       uint32_t buf[n],
                       uint32_t k)
{
    radix_sort(n, sa, rank, buf, k);
    radix_sort(n, sa, rank, buf, 0);
}

// For each element in sa, assumed sorted according to
// rank[sa[i]],rank[sa[i]+k], work out what rank
// (order of rank[sa[i]],rank[sa[i]+k]) each element has
// and put the result in out.
static uint32_t update_rank(uint32_t n,
                            uint32_t const sa[n],
                            uint32_t const rank[n],
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
void pd2(uint32_t n, char const x[n], uint32_t sa[n])
{
    uint32_t *buf = abort_malloc(n * sizeof *buf);
    uint32_t *rank = abort_malloc(n * sizeof *rank);

    uint32_t sigma = calc_rank0(n, x, rank);
    fill_indices(n, sa);

    for (uint32_t k = 1; sigma < n + 1; k *= 2)
    {
        sort_pairs(n, sa, rank, buf, k);
        sigma = update_rank(n, sa, rank, buf, k);
        SWAP(buf, rank); // get the rank back from buf to rank
    }

    free(rank);
    free(buf);
}
