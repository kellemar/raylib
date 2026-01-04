#include "minunit.h"
#include "../src/utils.h"
#include <stdlib.h>

static const char* test_clamp_within_range(void)
{
    mu_assert_float_eq(5.0f, ClampFloat(5.0f, 0.0f, 10.0f));
    return 0;
}

static const char* test_clamp_below_min(void)
{
    mu_assert_float_eq(0.0f, ClampFloat(-5.0f, 0.0f, 10.0f));
    return 0;
}

static const char* test_clamp_above_max(void)
{
    mu_assert_float_eq(10.0f, ClampFloat(15.0f, 0.0f, 10.0f));
    return 0;
}

static const char* test_clamp_at_boundaries(void)
{
    mu_assert_float_eq(0.0f, ClampFloat(0.0f, 0.0f, 10.0f));
    mu_assert_float_eq(10.0f, ClampFloat(10.0f, 0.0f, 10.0f));
    return 0;
}

static const char* test_spawn_interval_at_start(void)
{
    mu_assert_float_eq(2.0f, GetSpawnInterval(0.0f));
    return 0;
}

static const char* test_spawn_interval_decreases(void)
{
    float interval_at_50s = GetSpawnInterval(50.0f);
    mu_assert_float_eq(1.5f, interval_at_50s);
    return 0;
}

static const char* test_spawn_interval_minimum(void)
{
    mu_assert_float_eq(0.3f, GetSpawnInterval(200.0f));
    mu_assert_float_eq(0.3f, GetSpawnInterval(500.0f));
    return 0;
}

static const char* test_spawn_interval_at_170s(void)
{
    float interval = GetSpawnInterval(170.0f);
    mu_assert_float_eq(0.3f, interval);
    return 0;
}

static const char* test_vector2_distance_sq_same_point(void)
{
    Vector2 a = { 5.0f, 5.0f };
    Vector2 b = { 5.0f, 5.0f };
    mu_assert_float_eq(0.0f, Vector2DistanceSq(a, b));
    return 0;
}

static const char* test_vector2_distance_sq_horizontal(void)
{
    Vector2 a = { 0.0f, 0.0f };
    Vector2 b = { 3.0f, 0.0f };
    mu_assert_float_eq(9.0f, Vector2DistanceSq(a, b));
    return 0;
}

static const char* test_vector2_distance_sq_diagonal(void)
{
    Vector2 a = { 0.0f, 0.0f };
    Vector2 b = { 3.0f, 4.0f };
    mu_assert_float_eq(25.0f, Vector2DistanceSq(a, b));
    return 0;
}

static const char* test_circle_collision_overlapping(void)
{
    Vector2 c1 = { 0.0f, 0.0f };
    Vector2 c2 = { 5.0f, 0.0f };
    mu_assert_true(CheckCircleCollision(c1, 3.0f, c2, 3.0f));
    return 0;
}

static const char* test_circle_collision_touching(void)
{
    Vector2 c1 = { 0.0f, 0.0f };
    Vector2 c2 = { 6.0f, 0.0f };
    mu_assert_false(CheckCircleCollision(c1, 3.0f, c2, 3.0f));
    return 0;
}

static const char* test_circle_collision_not_touching(void)
{
    Vector2 c1 = { 0.0f, 0.0f };
    Vector2 c2 = { 10.0f, 0.0f };
    mu_assert_false(CheckCircleCollision(c1, 3.0f, c2, 3.0f));
    return 0;
}

static const char* test_circle_collision_same_center(void)
{
    Vector2 c1 = { 5.0f, 5.0f };
    Vector2 c2 = { 5.0f, 5.0f };
    mu_assert_true(CheckCircleCollision(c1, 1.0f, c2, 1.0f));
    return 0;
}

static const char* test_circle_collision_different_radii(void)
{
    Vector2 c1 = { 0.0f, 0.0f };
    Vector2 c2 = { 8.0f, 0.0f };
    mu_assert_true(CheckCircleCollision(c1, 5.0f, c2, 5.0f));
    mu_assert_false(CheckCircleCollision(c1, 3.0f, c2, 3.0f));
    return 0;
}

static const char* test_enemy_type_for_time_early(void)
{
    srand(12345);
    for (int i = 0; i < 100; i++)
    {
        int result = GetEnemyTypeForTime(15.0f);
        mu_assert_int_eq(0, result);
    }
    return 0;
}

static const char* test_enemy_type_for_time_mid(void)
{
    srand(12345);
    int hasChaser = 0;
    int hasOrbiter = 0;
    for (int i = 0; i < 200; i++)
    {
        int result = GetEnemyTypeForTime(45.0f);
        mu_assert(result >= 0 && result <= 1, "Mid-game should return 0 or 1");
        if (result == 0) hasChaser = 1;
        if (result == 1) hasOrbiter = 1;
    }
    mu_assert_true(hasChaser);
    mu_assert_true(hasOrbiter);
    return 0;
}

static const char* test_enemy_type_for_time_late(void)
{
    srand(12345);
    int hasChaser = 0;
    int hasOrbiter = 0;
    int hasSplitter = 0;
    for (int i = 0; i < 300; i++)
    {
        int result = GetEnemyTypeForTime(100.0f);
        mu_assert(result >= 0 && result <= 2, "Late-game should return 0, 1, or 2");
        if (result == 0) hasChaser = 1;
        if (result == 1) hasOrbiter = 1;
        if (result == 2) hasSplitter = 1;
    }
    mu_assert_true(hasChaser);
    mu_assert_true(hasOrbiter);
    mu_assert_true(hasSplitter);
    return 0;
}

// Edge case tests

static const char* test_clamp_negative_range(void)
{
    mu_assert_float_eq(-5.0f, ClampFloat(-5.0f, -10.0f, 0.0f));
    mu_assert_float_eq(-10.0f, ClampFloat(-15.0f, -10.0f, 0.0f));
    mu_assert_float_eq(0.0f, ClampFloat(5.0f, -10.0f, 0.0f));
    return 0;
}

static const char* test_clamp_very_large_values(void)
{
    mu_assert_float_eq(1000000.0f, ClampFloat(1500000.0f, 0.0f, 1000000.0f));
    mu_assert_float_eq(0.0f, ClampFloat(-1500000.0f, 0.0f, 1000000.0f));
    return 0;
}

static const char* test_clamp_very_small_range(void)
{
    mu_assert_float_eq(0.001f, ClampFloat(0.001f, 0.0f, 0.002f));
    mu_assert_float_eq(0.0f, ClampFloat(-0.001f, 0.0f, 0.002f));
    mu_assert_float_eq(0.002f, ClampFloat(0.003f, 0.0f, 0.002f));
    return 0;
}

static const char* test_clamp_zero_range(void)
{
    // When min == max, should return that value
    mu_assert_float_eq(5.0f, ClampFloat(3.0f, 5.0f, 5.0f));
    mu_assert_float_eq(5.0f, ClampFloat(7.0f, 5.0f, 5.0f));
    mu_assert_float_eq(5.0f, ClampFloat(5.0f, 5.0f, 5.0f));
    return 0;
}

static const char* test_vector2_distance_sq_negative_coords(void)
{
    Vector2 a = { -5.0f, -5.0f };
    Vector2 b = { -2.0f, -1.0f };
    // Distance: sqrt((3)^2 + (4)^2) = 5, so distSq = 25
    mu_assert_float_eq(25.0f, Vector2DistanceSq(a, b));
    return 0;
}

static const char* test_vector2_distance_sq_large_distance(void)
{
    Vector2 a = { 0.0f, 0.0f };
    Vector2 b = { 1000.0f, 1000.0f };
    // distSq = 1000^2 + 1000^2 = 2000000
    mu_assert_float_eq(2000000.0f, Vector2DistanceSq(a, b));
    return 0;
}

static const char* test_circle_collision_zero_radius(void)
{
    Vector2 c1 = { 0.0f, 0.0f };
    Vector2 c2 = { 0.0f, 0.0f };
    // Two points at same location with zero radius
    mu_assert_false(CheckCircleCollision(c1, 0.0f, c2, 0.0f));
    return 0;
}

static const char* test_circle_collision_one_contains_other(void)
{
    Vector2 c1 = { 0.0f, 0.0f };
    Vector2 c2 = { 1.0f, 1.0f };
    // Large circle contains small circle
    mu_assert_true(CheckCircleCollision(c1, 100.0f, c2, 1.0f));
    return 0;
}

static const char* test_circle_collision_negative_positions(void)
{
    Vector2 c1 = { -10.0f, -10.0f };
    Vector2 c2 = { -5.0f, -10.0f };
    // 5 units apart, radii sum to 6
    mu_assert_true(CheckCircleCollision(c1, 3.0f, c2, 3.0f));
    return 0;
}

static const char* test_spawn_interval_negative_time(void)
{
    // Game time should never be negative, but test graceful handling
    float interval = GetSpawnInterval(-10.0f);
    // Should treat as time 0 or handle gracefully
    mu_assert(interval >= 0.3f && interval <= 2.0f, "Spawn interval should be valid");
    return 0;
}

static const char* test_enemy_type_for_time_zero(void)
{
    srand(12345);
    for (int i = 0; i < 100; i++)
    {
        int result = GetEnemyTypeForTime(0.0f);
        mu_assert_int_eq(0, result);  // Only chasers at time 0
    }
    return 0;
}

static const char* test_enemy_type_for_time_very_late(void)
{
    srand(12345);
    int hasAllTypes = 0;
    int seenTypes[3] = { 0 };
    for (int i = 0; i < 500; i++)
    {
        int result = GetEnemyTypeForTime(1000.0f);  // Very late in game
        mu_assert(result >= 0 && result <= 2, "Should return valid type");
        seenTypes[result] = 1;
    }
    for (int i = 0; i < 3; i++)
    {
        if (seenTypes[i]) hasAllTypes++;
    }
    mu_assert_int_eq(3, hasAllTypes);  // Should see all enemy types
    return 0;
}

const char* run_utils_tests(void)
{
    mu_run_test(test_clamp_within_range);
    mu_run_test(test_clamp_below_min);
    mu_run_test(test_clamp_above_max);
    mu_run_test(test_clamp_at_boundaries);
    mu_run_test(test_clamp_negative_range);
    mu_run_test(test_clamp_very_large_values);
    mu_run_test(test_clamp_very_small_range);
    mu_run_test(test_clamp_zero_range);
    mu_run_test(test_spawn_interval_at_start);
    mu_run_test(test_spawn_interval_decreases);
    mu_run_test(test_spawn_interval_minimum);
    mu_run_test(test_spawn_interval_at_170s);
    mu_run_test(test_spawn_interval_negative_time);
    mu_run_test(test_vector2_distance_sq_same_point);
    mu_run_test(test_vector2_distance_sq_horizontal);
    mu_run_test(test_vector2_distance_sq_diagonal);
    mu_run_test(test_vector2_distance_sq_negative_coords);
    mu_run_test(test_vector2_distance_sq_large_distance);
    mu_run_test(test_circle_collision_overlapping);
    mu_run_test(test_circle_collision_touching);
    mu_run_test(test_circle_collision_not_touching);
    mu_run_test(test_circle_collision_same_center);
    mu_run_test(test_circle_collision_different_radii);
    mu_run_test(test_circle_collision_zero_radius);
    mu_run_test(test_circle_collision_one_contains_other);
    mu_run_test(test_circle_collision_negative_positions);
    mu_run_test(test_enemy_type_for_time_early);
    mu_run_test(test_enemy_type_for_time_mid);
    mu_run_test(test_enemy_type_for_time_late);
    mu_run_test(test_enemy_type_for_time_zero);
    mu_run_test(test_enemy_type_for_time_very_late);
    return 0;
}
