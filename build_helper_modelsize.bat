@echo off
rem run within source folder src\
rem https://docs.microsoft.com/en-us/cpp/build/reference/output-file-f-options?view=msvc-160

cls

set Mode=debug
set OutputFolder=debug

if not exist %OutputFolder% (
  mkdir %OutputFolder%
)

pushd %OutputFolder%

set WinLibs=kernel32.lib User32.lib Gdi32.lib shell32.lib vcruntime.lib winmm.lib
set ExternalLibs=
set IncludePaths=/I..\..\include
set GenerateCompleteDebuggingInfo=/Zi
set WarningLevel=/W4
set IWPadding=/wd4820
set IWInitializedNotReferenced=/wd4189
set IWUnusedParam=/wd4100
set IWNamelessStructUnion=/wd4201
set IgnoreWarnings=%IWPadding% %IWInitializedNotReferenced% %IWUnusedParam% %IWNamelessStructUnion%


set dllname=win32_modelsizes
cl /nologo ^
  %WarningLevel% ^
  %IgnoreWarnings% ^
  %GenerateCompleteDebuggingInfo% ^
  %CompilationFlags% ^
  %IncludePaths% ^
  ..\%dllname%.cpp  ^
  /Fm%dllname%.map ^
  /link ^
  /PDB:%dllname%.pdb ^
  /SUBSYSTEM:CONSOLE ^
  %WinLibs%

popd

