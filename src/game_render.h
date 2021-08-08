#ifndef GAME_RENDER_H

#define DEFAULT_WORLD_UP V3(0,1,0)
#include "game_math.h"
#include "game_entity.h"

GAME_API void
BeginRender(game_state * GameState, v4 ClearColor);

GAME_API void
EndRender(game_state * GameState);

GAME_API inline void 
Translate(m4 &M,v3 P);

GAME_API void
RenderEntities(game_memory * Memory, game_state * GameState);


GAME_API void
WorldInitializeView(game_state * GameState,
               real32 FOV,
               int32 ScreenWidth, int32 ScreenHeight, 
               real32 n, real32 f, 
               v3 P, v3 WorldUp = DEFAULT_WORLD_UP);

GAME_API void 
RotateFill(m4 * M, real32 AngleX, real32 AngleY, real32 AngleZ);

GAME_API void
MoveViewRight(game_state * GameState,real32 N);

GAME_API void
MoveViewForward(game_state * GameState,real32 N);


GAME_API inline v3
GetViewPos(game_state * GameState);

GAME_API inline v3
GetMatrixRight(m4 &RotationMatrix);

GAME_API inline v3
GetMatrixPos(m4 &M);

GAME_API inline v3
GetMatrixDirection(m4 &RotationMatrix);

GAME_API int32
ViewLookAt(game_state * GameState, v3 P, v3 TargetP);

GAME_API void
EntityLookAt(game_state * GameState,entity Entity, v3 P);

GAME_API m4
ProjectionMatrix(real32 FOV,real32 AspectRatio, real32 n, real32 f);

#define GAME_RENDER_H
#endif
