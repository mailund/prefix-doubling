#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pd-3.h"
#include "shared.h"

// Sort the elements in one sa bucket according to the rank[sa[i]+k]
// (with padded zero sentinels) using a radix sort over
// 8-bit sub-integers. The result is left in sa; buf
// is a scratch buffer.
static void radix_sort_bucket(uint32_t n, uint32_t m, uint32_t k,
                              uint32_t const rank[n],
                              uint32_t sa[m],
                              uint32_t buf[m])
{
    static uint32_t buckets[256]; // for sorting bytes

    // Radix sort over the four bytes in uint32_t. We flip buffers along the way,
    // but since we do an even number of bucket sorts we end up with the result
    // back in sa.
    for (uint32_t shift = 0; shift < 32; shift += 8)
    {
        memset(buckets, 0, sizeof buckets);

        // compute bucket offsets
        for (uint32_t i = 0; i < m; i++)
        {
            uint8_t b = get_rank(n, rank, sa[i] + k) >> shift;
            buckets[b]++;
        }
        for (uint32_t acc = 0, i = 0; i < 256; i++)
        {
            uint32_t b = buckets[i];
            buckets[i] = acc;
            acc += b;
        }
        // then place sa[i] in buckets
        for (uint32_t i = 0; i < m; i++)
        {
            uint8_t b = get_rank(n, rank, sa[i] + k) >> shift;
            buf[buckets[b]++] = sa[i];
        }

        SWAP(sa, buf); // flip sa and buf for next iteration...
    }
}

// Sort the elements sa according to the rank[sa[i]+k]
// (with padded zero sentinels) using a radix sort over
// 8-bit sub-integers. The result is left in sa; buf
// is a scratch buffer.
static void radix_sort(uint32_t n, uint32_t k,
                       uint32_t const rank[n],
                       uint32_t sa[n],
                       uint32_t buf[n])
{
    // sa is already sorted, so we need to sort sa+k for each bucket.
    uint32_t b_start = 0, b_end = 0;
    while (b_start < n)
    {
        while (b_end < n && rank[sa[b_start]] == rank[sa[b_end]])
            b_end++;

        // Sort the bucket if it is more than one element large
        if ((b_end - b_start) > 1)
            radix_sort_bucket(n, b_end - b_start, k, rank, sa + b_start, buf + b_start);

        b_start = b_end;
    }
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
void pd3(uint32_t n, char const x[n], uint32_t sa[n]) // FlawFinder: ignore (x[n] is fine)
{
    uint32_t *buf = abort_malloc(n * sizeof *buf);
    uint32_t *rank = abort_malloc(n * sizeof *rank);

    uint32_t sigma = calc_rank0(n, x, rank);
    fill_indices(n, sa);

    // start by sorting according to the first rank.
    radix_sort_bucket(n, n, 0, rank, sa, buf);

    for (uint32_t k = 1; sigma < n + 1; k *= 2)
    {
        radix_sort(n, k, rank, sa, buf);
        sigma = update_rank(n, sa, rank, buf, k);
        SWAP(buf, rank); // get the rank back from buf to rank
    }

    free(rank);
    free(buf);
}
