
#include "game_world.h"
#include "game_memory.h"
#include "game_mesh.h"

struct async_load_ground
{
    world * World;
    thread_memory_arena * ThreadArena;
    mesh_group  * MeshGroup;

    memory_arena * VertexArena;
    world_pos WorldP;
    i32 MaxTileX;
    i32 MaxTileY;
    i32 MaxTileZ;
    u32 VoxelsPerAxis;
    game_state * GameState;
};
