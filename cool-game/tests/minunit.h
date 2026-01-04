#ifndef MINUNIT_H
#define MINUNIT_H

#include <stdio.h>
#include <math.h>

extern int mu_tests_run;
extern int mu_tests_passed;
extern int mu_asserts;

#ifdef MINUNIT_MAIN
int mu_tests_run = 0;
int mu_tests_passed = 0;
int mu_asserts = 0;
#endif

#define MU_EPSILON 0.0001f

#define mu_assert(test, message) \
    do { \
        mu_asserts++; \
        if (!(test)) { \
            printf("  FAIL: %s\n", message); \
            printf("        at %s:%d\n", __FILE__, __LINE__); \
            return message; \
        } \
    } while (0)

#define mu_assert_int_eq(expected, actual) \
    do { \
        mu_asserts++; \
        int _e = (expected); \
        int _a = (actual); \
        if (_e != _a) { \
            printf("  FAIL: expected %d, got %d\n", _e, _a); \
            printf("        at %s:%d\n", __FILE__, __LINE__); \
            return "int equality assertion failed"; \
        } \
    } while (0)

#define mu_assert_float_eq(expected, actual) \
    do { \
        mu_asserts++; \
        float _e = (expected); \
        float _a = (actual); \
        if (fabsf(_e - _a) > MU_EPSILON) { \
            printf("  FAIL: expected %.4f, got %.4f\n", _e, _a); \
            printf("        at %s:%d\n", __FILE__, __LINE__); \
            return "float equality assertion failed"; \
        } \
    } while (0)

#define mu_assert_true(actual) \
    mu_assert((actual), "expected true, got false")

#define mu_assert_false(actual) \
    mu_assert(!(actual), "expected false, got true")

#define mu_run_test(test) \
    do { \
        printf("  %s... ", #test); \
        const char *message = test(); \
        mu_tests_run++; \
        if (message) { \
            return message; \
        } else { \
            mu_tests_passed++; \
            printf("OK\n"); \
        } \
    } while (0)

#define mu_run_suite(name, suite) \
    do { \
        printf("\n[%s]\n", name); \
        const char *result = suite(); \
        if (result != 0) { \
            printf("\nSuite failed: %s\n", result); \
        } \
    } while (0)

#define mu_print_summary() \
    do { \
        printf("\n========================================\n"); \
        printf("Tests: %d passed, %d failed, %d total\n", \
               mu_tests_passed, mu_tests_run - mu_tests_passed, mu_tests_run); \
        printf("Assertions: %d\n", mu_asserts); \
        printf("========================================\n"); \
    } while (0)

#define mu_success() (mu_tests_run == mu_tests_passed)

#endif
