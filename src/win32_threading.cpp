#include "game_platform.h"
#include <windows.h>


#ifndef PLATFORM_THREADING
#define PLATFORM_THREADING
#define THREAD_START_ROUTINE(name) DWORD WINAPI name(void * Data)
typedef THREAD_START_ROUTINE(thread_start_routine);
#endif


THREAD_DO_WORK_ON_QUEUE(DoWorkOnQueue)
{
    b32 GoToSleep = false;

    u32 CurrentRead = Queue->CurrentRead;
    u32 NextReadEntry = (CurrentRead + 1) % ArrayCount(Queue->Entries);

    if (CurrentRead != Queue->CurrentWrite)
    {
        if (CompareAndExchangeIfMatches(&Queue->CurrentRead, CurrentRead, NextReadEntry))
        {
            thread_work_queue_entry Entry = Queue->Entries[CurrentRead];
            Entry.Handler(Queue, Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->ThingsDone);
        }
    }
    else
    {
        GoToSleep = true;
    }


    return GoToSleep;
}

THREAD_COMPLETE_QUEUE(CompleteWorkQueue)
{
    while (Queue->ThingsToDo != Queue->ThingsDone)
    {
        DoWorkOnQueue(Queue);
    }
    Queue->ThingsToDo = 0;
    Queue->ThingsDone = 0;
}

#ifdef GAME_MULTITHREAD
THREAD_ADD_WORK_TO_QUEUE(AddWorkToQueue)
{
    u32 NextWriteEntry = (Queue->CurrentWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NextWriteEntry != Queue->CurrentRead);

    thread_work_queue_entry * Entry = (Queue->Entries + Queue->CurrentWrite);
    Entry->Handler = Handler;
    Entry->Data = Data;

    MemoryBarrier();
    Queue->CurrentWrite = NextWriteEntry;
    ++Queue->ThingsToDo;

    ReleaseSemaphore(Queue->Semaphore, 1, 0);
}
#else
THREAD_ADD_WORK_TO_QUEUE(AddWorkToQueue)
{
    u32 NextWriteEntry = (Queue->CurrentWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NextWriteEntry != Queue->CurrentRead);

    thread_work_queue_entry * Entry = (Queue->Entries + Queue->CurrentWrite);
    Entry->Handler = Handler;
    Entry->Data = Data;

    MemoryBarrier();
    Queue->CurrentWrite = NextWriteEntry;
    ++Queue->ThingsToDo;

    Queue->DoWorkOnQueue(Queue);
}
#endif

THREAD_START_ROUTINE(WorkQueueThreadLoop)
{
    thread_work_queue * Queue = (thread_work_queue *)Data;
    for (;;)
    {
        if (Queue->DoWorkOnQueue(Queue))
        {
            WaitForSingleObjectEx(Queue->Semaphore, INFINITE, FALSE);
        }
    }
}

void
CleanWorkQueue(thread_work_queue * Queue)
{
    if (Queue->Semaphore)
    {
        CloseHandle(Queue->Semaphore);
        Queue->Semaphore = 0;
    }
    Queue->CurrentWrite = 0;
    Queue->CurrentRead = 0;

    Queue->ThingsDone = 0;
    Queue->ThingsToDo = 0;
}

void
CreateWorkQueue(thread_work_queue * Queue, u32 CountThreads, thread_do_work_on_queue * ThreadDoWorkOnQueue = DoWorkOnQueue)
{
    SECURITY_ATTRIBUTES * NullSecAttrib = 0;
    const char * NullName               = 0;
    DWORD NullReservedFlags             = 0;
    i32 SizeStack                     = Megabytes(1);
    DWORD RunInmediate                  = 0;
    DWORD ThreadID;

    CleanWorkQueue(Queue);
    void * ThreadParam = (void *)Queue;

    DWORD DesiredAccess = SEMAPHORE_ALL_ACCESS; 
    HANDLE Semaphore = CreateSemaphoreExA( NullSecAttrib, 0, CountThreads, NullName, NullReservedFlags, DesiredAccess);
    Queue->Semaphore = Semaphore;
    Queue->DoWorkOnQueue = ThreadDoWorkOnQueue;

    for (u32 ThreadIndex = 0;
                ThreadIndex < CountThreads;
                ++ThreadIndex)
    {
        HANDLE T = CreateThread(NullSecAttrib, SizeStack, 
                                WorkQueueThreadLoop, ThreadParam, RunInmediate,
                                &ThreadID);
        Assert(T);
        // Close right away. 2 things need to happen to clean up thread:
        // 1) handle is closed
        // 2) thread function reaches end
        CloseHandle(T);
    }
}
