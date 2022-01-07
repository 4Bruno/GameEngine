#ifndef GAME_RENDER_H

#define DEFAULT_WORLD_UP V3(0,1,0)
#include "game_math.h"
#include "game_entity.h"

enum  shader_type_vertex
{
    shader_type_vertex_default,


    shader_type_vertex_total
};

enum  shader_type_fragment
{
    shader_type_fragment_default,


    shader_type_fragment_total
};

GAME_API void
CreatePipeline(game_memory * Memory,game_state * GameState);

GAME_API void
EntityAdd3DRender(game_state * GameState, entity Entity, u32 MeshID, v3 Color);

GAME_API void
BeginRender(game_state * GameState, v4 ClearColor);

GAME_API void
EndRender(game_state * GameState);

GAME_API void
RenderEntities(game_memory * Memory, game_state * GameState);

GAME_API void
RenderGround(game_state * GameState, entity * Entity);


GAME_API void
WorldInitializeView(game_state * GameState,
               r32 FOV,
               i32 ScreenWidth, i32 ScreenHeight, 
               r32 n, r32 f, 
               v3 P, v3 WorldUp = DEFAULT_WORLD_UP);

GAME_API void 
RotateFill(m4 * M, r32 AngleX, r32 AngleY, r32 AngleZ);

GAME_API void
MoveViewRight(game_state * GameState,r32 N);

GAME_API void
MoveViewForward(game_state * GameState,r32 N);


GAME_API inline v3
GetViewPos(game_state * GameState);

GAME_API inline v3
GetMatrixRight(m4 &RotationMatrix);

GAME_API inline v3
GetMatrixPos(m4 &M);

GAME_API inline v3
GetMatrixDirection(m4 &RotationMatrix);

i32
ViewLookAt(game_state * GameState, v3 P, v3 TargetP);

GAME_API void
EntityLookAt(game_state * GameState,entity Entity, v3 P);

m4
ProjectionMatrix(r32 FOV,r32 AspectRatio, r32 n, r32 f);


#define GAME_RENDER_H
#endif
