#include "utils.h"

float ClampFloat(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float GetSpawnInterval(float gameTime)
{
    float interval = 2.0f - gameTime * 0.01f;
    return ClampFloat(interval, 0.3f, 2.0f);
}

float Vector2DistanceSq(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

bool CheckCircleCollision(Vector2 c1, float r1, Vector2 c2, float r2)
{
    float radiusSum = r1 + r2;
    return Vector2DistanceSq(c1, c2) < radiusSum * radiusSum;
}
