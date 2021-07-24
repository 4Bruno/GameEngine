#ifndef DATA_LOAD_H
#include "game_platform.h"
#include "game_memory.h"

struct file_contents
{
    uint8 * Base;
    uint32 Size;
    bool32 Success;
};

file_contents
GetFileContents(game_memory * GameMemory,memory_arena * Arena,const char * Filepath);


#define DATA_LOAD_H
#endif
