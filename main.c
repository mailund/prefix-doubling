#include <stdio.h>

#include "pd-1.h"
#include "pd-2.h"
#include "pd-3.h"
#include "shared.h"

int main(void)
{
    uint32_t n = 1000000;
    char *x = abort_malloc(n + 1); // n + 1 for '\0'
    uint32_t *sa = abort_malloc(n * sizeof *sa);

    random_string(n, x);

    char const *func_names[] = {"pd1", "pd2", "pd3"};
    construction_func funcs[] = {pd1, pd2, pd3};
    int no_funcs = sizeof func_names / sizeof *func_names;
    performance_measurements(no_funcs,
                             func_names,
                             funcs, n, 10);

    free(x);
    free(sa);

    return 0;
}