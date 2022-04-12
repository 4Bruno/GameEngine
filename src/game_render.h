#ifndef GAME_RENDER_H

#define DEFAULT_WORLD_UP V3(0,1,0)
#include "game_math.h"
#include "game_entity.h"
#include "graphics_api.h"

enum projection_mode
{
    projection_perspective,
    projection_orthographic
};

GAME_API void
PushDrawSimulation(render_controller * Renderer,world * World, simulation * Sim);

GAME_API void
BeginRenderPass(v4 ClearColor, v4 AmbientLight, v4 SunlightDirection, v4 SunlightColor);

GAME_API void
BeginRender(render_controller * Renderer,i32 ScreenWidth, i32 ScreenHeight);

GAME_API void
RenderDraw(render_controller * Renderer);

GAME_API void
EndRender();

#if DEBUG
GAME_API void
PushDrawDebug(render_controller * Renderer,entity * Entity);
#endif

GAME_API void
PushDrawEntity(render_controller * Renderer,entity * Entity);
GAME_API void
PushDraw(render_controller * Renderer, game_asset_id Material, m4 * ModelT, game_asset_id MeshID, game_asset_id TextureID, v3 Color, r32 Transparency);
GAME_API void
PushDrawParticle(render_controller * Renderer, m4 * ModelT, mesh_group * MeshGroup, game_asset_id TextureID, v3 Color, r32 Transparency);

GAME_API render_controller
NewRenderController(memory_arena * Arena, u32 RenderUnitLimits,
                    v3 WorldUp,
                    r32 FOV,
                    i32 ScreenWidth, i32 ScreenHeight, 
                    r32 n, r32 f,
                    v3 StartP,
                    projection_mode ProjectionMode);

GAME_API void 
RotateFill(m4 * M, r32 AngleX, r32 AngleY, r32 AngleZ);

GAME_API void
MoveViewRight(render_controller * Renderer,r32 N);

GAME_API void
MoveViewForward(render_controller * Renderer,r32 N);


GAME_API inline v3
GetViewPos(render_controller * Renderer);

GAME_API inline v3
GetMatrixRight(m4 &RotationMatrix);

GAME_API inline v3
GetMatrixPos(m4 &M);

GAME_API inline v3
GetMatrixDirection(m4 &RotationMatrix);

i32
ViewLookAt(render_controller * Renderer, v3 P, v3 TargetP);
i32
LookAt(Quaternion * Q, v3 P, v3 TargetP, v3 WorldUp);
i32
LookAt(m4 * M, v3 P, v3 TargetP, v3 WorldUp);

GAME_API void
EntityLookAt(render_controller * Renderer,entity Entity, v3 P);

m4
ProjectionMatrix(r32 FOV,r32 AspectRatio, r32 n, r32 f);

//GAME_API void TestGroundGPU(game_memory * Memory,memory_arena * TempArena);

#define GAME_RENDER_H
#endif
