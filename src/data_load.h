#ifndef DATA_LOAD_H
#include "game_platform.h"
#include "game_memory.h"

struct file_contents
{
    u8 * Base;
    u32 Size;
    b32 Success;
};

GAME_API file_contents
GetFileContents(game_memory * GameMemory,memory_arena * Arena,const char * Filepath);


#define DATA_LOAD_H
#endif
