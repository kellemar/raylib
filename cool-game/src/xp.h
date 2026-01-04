#ifndef XP_H
#define XP_H

#include "raylib.h"
#include "types.h"
#include <stdbool.h>

typedef struct XPCrystal {
    Vector2 pos;
    int value;
    float radius;
    float lifetime;
    bool active;
    int activeIndex;
} XPCrystal;

typedef struct XPPool {
    XPCrystal crystals[MAX_XP_CRYSTALS];
    int activeIndices[MAX_XP_CRYSTALS];
    int freeIndices[MAX_XP_CRYSTALS];
    int freeCount;
    int count;
} XPPool;

void XPPoolInit(XPPool *pool);
void XPPoolUpdate(XPPool *pool, Vector2 playerPos, float magnetRadius, float dt);
void XPPoolDraw(XPPool *pool, Rectangle view);
void XPSpawn(XPPool *pool, Vector2 pos, int value);
int XPCollect(XPPool *pool, Vector2 playerPos, float collectRadius);

#endif
