@echo off
cls

pushd debug

set WinLibs=kernel32.lib User32.lib Gdi32.lib shell32.lib libvcruntime.lib winmm.lib
set CompilationFlags=%BUILD_COMPILATION_FLAGS% /DDEBUG=1
set ExternalLibs=
set IncludePaths=/I..\..\include /I..\..\libs
set GenerateCompleteDebuggingInfo=/Zi
set WarningLevel=/W4
set IWPadding=/wd4820
set IWInitializedNotReferenced=/wd4189
set IWUnusedParam=/wd4100
set IWNamelessStructUnion=/wd4201
set IgnoreWarnings=%IWPadding% %IWInitializedNotReferenced% %IWUnusedParam% %IWNamelessStructUnion%

set compilation_vars=%WarningLevel% %IgnoreWarnings% %GenerateCompleteDebuggingInfo% %CompilationFlags% %IncludePaths%

set dllname=test_assets_to_gpu
erase %dllname%*
cl /nologo %compilation_vars% ..\%dllname%.cpp  /Fm%dllname%.map /link /PDB:%dllname%.pdb /SUBSYSTEM:CONSOLE %WinLibs%

rem set dllname=win32_platform
rem erase %dllname%*
rem cl /nologo %compilation_vars% ..\%dllname%.cpp ..\hierarchy_tree.cpp ..\game_memory.cpp  ..\gpu_heap.cpp /Fm%dllname%.map /link /PDB:%dllname%.pdb /SUBSYSTEM:CONSOLE %WinLibs%

popd
