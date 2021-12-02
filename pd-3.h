#ifndef PD_3_H
#define PD_3_H

#include <stdint.h>

void pd3(uint32_t n, char const x[n], uint32_t sa[n]); // FlawFinder: ignore (x[n] is fine)

#endif
