#ifndef PD_2_H
#define PD_2_H

#include <stdint.h>

void pd2(uint32_t n, char const x[n], uint32_t sa[n]); // FlawFinder: ignore (x[n] is fine)

#endif
