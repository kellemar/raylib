#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>

#ifndef RAYLIB_H
typedef struct Vector2 {
    float x;
    float y;
} Vector2;
#endif

float ClampFloat(float value, float min, float max);
float GetSpawnInterval(float gameTime);
float Vector2DistanceSq(Vector2 a, Vector2 b);
bool CheckCircleCollision(Vector2 c1, float r1, Vector2 c2, float r2);

#endif
