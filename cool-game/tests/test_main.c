#include <stdio.h>
#define MINUNIT_MAIN
#include "minunit.h"

extern const char* run_utils_tests(void);
extern const char* run_enemy_tests(void);
extern const char* run_projectile_tests(void);
extern const char* run_xp_tests(void);
extern const char* run_weapon_tests(void);
extern const char* run_upgrade_tests(void);

int main(void)
{
    printf("NEON VOID - Unit Tests\n");
    printf("========================================\n");

    mu_run_suite("Utils", run_utils_tests);
    mu_run_suite("Enemy", run_enemy_tests);
    mu_run_suite("Projectile", run_projectile_tests);
    mu_run_suite("XP", run_xp_tests);
    mu_run_suite("Weapon", run_weapon_tests);
    mu_run_suite("Upgrade", run_upgrade_tests);

    mu_print_summary();

    return mu_success() ? 0 : 1;
}
