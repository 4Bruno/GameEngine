@echo off
rem run within source folder src\
rem https://docs.microsoft.com/en-us/cpp/build/reference/output-file-f-options?view=msvc-160

cls

set Mode=release
set OutputFolder=release
if !%1!==!debug! (
    set Mode=debug
    set OutputFolder=debug
)

set WinLibs=kernel32.lib User32.lib Gdi32.lib shell32.lib vcruntime.lib
set CompilationFlags=/DVK_USE_PLATFORM_WIN32_KHR=1 /D_WIN32=1
set ExternalLibs=
set IncludePaths=/I..\include
set GenerateCompleteDebuggingInfo=/Zi
set WarningLevel=/W4
set IWPadding=/wd4820
set IWInitializedNotReferenced=/wd4189
set IWUnusedParam=/wd4100
set IWNamelessStructUnion=/wd4201
set IgnoreWarnings=%IWPadding% %IWInitializedNotReferenced% %IWUnusedParam% %IWNamelessStructUnion%

if not exist %OutputFolder% (
  mkdir %OutputFolder%
)

set dllname=render
cl /nologo ^
  /LD ^
  %WarningLevel% ^
  %IgnoreWarnings% ^
  %GenerateCompleteDebuggingInfo% ^
  %CompilationFlags% ^
  %IncludePaths% ^
  %dllname%.cpp ^
  /Fe%OutputFolder%\%dllname%.dll ^
  /Fo%OutputFolder%\%dllname%.obj ^
  /Fd%OutputFolder%\%dllname%.pdb ^
  /link ^
  %ExternalLibs%

set dllname=win32_platform
cl /nologo ^
  %WarningLevel% ^
  %IgnoreWarnings% ^
  %GenerateCompleteDebuggingInfo% ^
  %CompilationFlags% ^
  %IncludePaths% ^
  %dllname%.cpp ^
  /Fe%OutputFolder%\%dllname%.exe ^
  /Fo%OutputFolder%\%dllname%.obj ^
  /Fd%OutputFolder%\%dllname%.pdb ^
  /link ^
  /SUBSYSTEM:CONSOLE ^
  %WinLibs% %OutputFolder%\render.lib

set dllname=game
cl /nologo ^
  /LD ^
  %WarningLevel% ^
  %IgnoreWarnings% ^
  %GenerateCompleteDebuggingInfo% ^
  %CompilationFlags% ^
  %IncludePaths% ^
  %dllname%.cpp ^
  /Fe%OutputFolder%\%dllname%.dll ^
  /Fo%OutputFolder%\%dllname%.obj ^
  /Fd%OutputFolder%\%dllname%.pdb ^
  /link ^
  %OutputFolder%\render.lib
