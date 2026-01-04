#include <stdio.h>
#define MINUNIT_MAIN
#include "minunit.h"

extern const char* run_utils_tests(void);
extern const char* run_enemy_tests(void);

int main(void)
{
    printf("NEON VOID - Unit Tests\n");
    printf("========================================\n");

    mu_run_suite("Utils", run_utils_tests);
    mu_run_suite("Enemy", run_enemy_tests);

    mu_print_summary();

    return mu_success() ? 0 : 1;
}
