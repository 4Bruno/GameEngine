#ifndef GAME_ANIMATION_H

#include "game_platform.h"
#include "Quaternion.h"

struct animation
{
    b32 Started;
    r32 t;

    r32 StepCount;
    r32 StepSize;
    r32 _TimeStep;
    b32 Interrumpible;

    Quaternion StartRotation;
    Quaternion EndRotation;
};



#define GAME_ANIMATION_H
#endif
