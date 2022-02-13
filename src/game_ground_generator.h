
#include "game_world.h"
#include "game_memory.h"
#include "game_mesh.h"

struct async_load_ground
{
    world * World;
    thread_memory_arena * ThreadArena;
    mesh_group  * MeshGroup;

    void * FragmentedBuffer;

    world_pos WorldP;
    u32 VoxelsPerAxis;
};
