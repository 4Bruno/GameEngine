#ifndef RENDER_H
#include "game_math.h"
#include "game.h"
#include "game_memory.h"
#include "vulkan_initializer.h"
#include "data_load.h"
#include "collision.h"



inline void Translate(m4 &M,v3 P);













void
RotateEntity(game_state * GameState,entity Entity,real32 Pitch, real32 Yaw)
{
    real32 cx = cosf(Pitch);
    real32 sx = sinf(Pitch);
    real32 cy = cosf(Yaw);
    real32 sy = sinf(Yaw);

    v3 xaxis = { cy, 0, -sy};
    v3 yaxis = { sy* sx, cx, cy * sx};
    v3 zaxis = { sy* cx, -sx, cx * cy};

    m4 R =  {
        V4( xaxis.x, yaxis.x, zaxis.x, 0 ),
        V4( xaxis.y, yaxis.y, zaxis.y, 0 ),
        V4( xaxis.z, yaxis.z, zaxis.z, 0 ),
        V4( 0,0,0,1 )
    };

    entity_transform * T = GetEntityTransform(GameState,Entity);

    T->LocalR = R * T->LocalR;
}






void
RotateRight(m4 * M,real32 Angle)
{
    real32 c = cosf(-Angle);
    real32 s = sinf(-Angle);
    m4 R = {
        c, 0 , s , 0,
        0, 1, 0 , 0,
        -s, 0 , c, 0,
        0, 0, 0, 1
    };

    *M = R * (*M);
}

void
RotateUp(m4 * M,real32 Angle)
{
    real32 c = cosf(-Angle);
    real32 s = sinf(-Angle);
    m4 R = {
        1, 0 , 0, 0,
        0, c, -s, 0,
        0, s, c, 0,
        0, 0, 0, 1
    };

    *M = R * (*M);
}

void
SetViewBehindObject(game_state * GameState, v3 T, v3 D, real32 Separation, real32 HeightOverObject = 0.0f)
{
    D = (D / Length(D));

    v3 V = T - D * Separation + V3(0,HeightOverObject,0);

    ViewLookAt(GameState,V,T);
}

#define RENDER_H
#endif
