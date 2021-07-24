@echo off
rem run within source folder src\
rem https://docs.microsoft.com/en-us/cpp/build/reference/output-file-f-options?view=msvc-160

cls

set Mode=release
set OutputFolder=release
if !%1!==!debug! (
    set Mode=debug
    set OutputFolder=debug
    rem VS locks .pdb files. Create dummy _temp and in win32_platform
    rem we have logic to pickup temp file and copy it
    set Dllreload=_temp
    set BUILD_COMPILATION_FLAGS=/DDEBUG=1
)

if not exist %OutputFolder% (
  mkdir %OutputFolder%
)

pushd %OutputFolder%

erase game_*.pdb

set WinLibs=kernel32.lib User32.lib Gdi32.lib shell32.lib vcruntime.lib winmm.lib
set CompilationFlags=%BUILD_COMPILATION_FLAGS%
set ExternalLibs=
set IncludePaths=/I..\..\include
set GenerateCompleteDebuggingInfo=/Zi
set WarningLevel=/W4
set IWPadding=/wd4820
set IWInitializedNotReferenced=/wd4189
set IWUnusedParam=/wd4100
set IWNamelessStructUnion=/wd4201
set IgnoreWarnings=%IWPadding% %IWInitializedNotReferenced% %IWUnusedParam% %IWNamelessStructUnion%

set VulkanLib=vulkan_initializer
set dllname=%VulkanLib%
cl /nologo ^
  /LD ^
  %WarningLevel% ^
  %IgnoreWarnings% ^
  %GenerateCompleteDebuggingInfo% ^
  %CompilationFlags% ^
  %IncludePaths% ^
  ..\%dllname%.cpp ^
  /link ^
  /DLL ^
  /incremental:no /opt:ref /PDB:%VulkanLib%.pdb ^
  %ExternalLibs%

set dllname=win32_platform
cl /nologo ^
  %WarningLevel% ^
  %IgnoreWarnings% ^
  %GenerateCompleteDebuggingInfo% ^
  %CompilationFlags% ^
  %IncludePaths% ^
  ..\%dllname%.cpp ^
  /Fm%dllname%.map ^
  /link ^
  /PDB:%dllname%.pdb ^
  /SUBSYSTEM:CONSOLE ^
  %WinLibs% %VulkanLib%.lib

set dllname=game
set dllnameOutput=%dllname%%Dllreload%
cl /nologo ^
  /LD /MTd ^
  /Fm%dllname%.map ^
  %WarningLevel% ^
  %IgnoreWarnings% ^
  %GenerateCompleteDebuggingInfo% ^
  %CompilationFlags% ^
  %IncludePaths% ^
  ..\%dllname%.cpp ..\data_load.cpp ^
  /Fe:%dllnameOutput%.dll ^
  /link ^
  /DLL ^
  /incremental:no /opt:ref /PDB:%dllnameOutput%_%random%.pdb ^
  %VulkanLib%.lib

popd

