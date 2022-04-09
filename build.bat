@echo off
rem run within source folder src\
rem https://docs.microsoft.com/en-us/cpp/build/reference/output-file-f-options?view=msvc-160
setlocal ENABLEDELAYEDEXPANSION
rem Use wrapper for comparison ""
rem Inside of IF statements use !! to force setting variables before using them

cls

set Mode=release
set OutputFolder=release
if "%1"=="debug" (
    set Mode=debug
    set OutputFolder=debug
    rem VS locks .pdb files. Create dummy _temp and in win32_platform
    rem we have logic to pickup temp file and copy it
    set Dllreload=_temp
    set BUILD_COMPILATION_FLAGS=/DDEBUG=1
)

set BuildDllID=All
if "%2" NEQ "" (
    set BuildDllID=%2
)
echo Build request: %BuildDllID%

if not exist %OutputFolder% (
  mkdir %OutputFolder%
)

pushd %OutputFolder%


set WinLibs=kernel32.lib User32.lib Gdi32.lib shell32.lib vcruntime.lib winmm.lib
set CompilationFlags=%BUILD_COMPILATION_FLAGS%
if "%Mode%"=="release"  (
    set CompilationFlags=%CompilationFlags% /O2
)
set ExternalLibs=
set IncludePaths=/I..\..\include
set GenerateCompleteDebuggingInfo=/Zi
set WarningLevel=/W4
set IWPadding=/wd4820
set IWInitializedNotReferenced=/wd4189
set IWUnusedParam=/wd4100
set IWNamelessStructUnion=/wd4201
set IgnoreWarnings=%IWPadding% %IWInitializedNotReferenced% %IWUnusedParam% %IWNamelessStructUnion%

set BuildThis=Y
if "%BuildDllID%" NEQ "All" (
    if "%BuildDllID%" NEQ "1" (
        set BuildThis=N
    )
)
if "%BuildThis%"=="Y"  (

    set graphicsDllName=graphics_api
    set graphicsOutput=!graphicsDllName!!Dllreload!
    set GraphicsLib=graphics_api
    set dllname=!GraphicsLib!

    echo Building graphics DLL !dllname!
    erase !dllname!*

    cl /nologo ^
     /LD !WarningLevel! !IgnoreWarnings! !GenerateCompleteDebuggingInfo! !CompilationFlags! !IncludePaths! ^
     ..\!dllname!.cpp  ..\vulkan_helpers.cpp   ^
     /Fe:!graphicsOutput!.dll ^
     /link /DLL ^
      /incremental:no /opt:ref /PDB:!graphicsOutput!_!random!.pdb ^
      !ExternalLibs!

)

set BuildThis=Y
if "%BuildDllID%" NEQ "All" (
    if "%BuildDllID%" NEQ "2" (
        set BuildThis=N
    )
)
if "%BuildThis%"=="Y" (
    set dllname=win32_platform
    erase !dllname!*
    cl /nologo ^
     !WarningLevel! !IgnoreWarnings! !GenerateCompleteDebuggingInfo! !CompilationFlags! !IncludePaths! ..\!dllname!.cpp ^
      /Fm!dllname!.map ^
      /link ^
      /PDB:!dllname!.pdb ^
      /SUBSYSTEM:CONSOLE ^
      !WinLibs!
)

set BuildThis=Y
if "%BuildDllID%" NEQ "All" (
    if "%BuildDllID%" NEQ "3" (
        set BuildThis=N
    )
)
if "%BuildThis%"=="Y" (

    set dllname=game
    set dllnameOutput=!dllname!!Dllreload!
    rem ..\data_load.cpp ..\game_memory.cpp ..\game_entity.cpp ..\Quaternion.cpp ..\game_render.cpp ..\game_mesh.cpp ..\game_world.cpp ..\game_simulation.cpp ^

    echo Building Game DLL !dllname!

    erase !dllname!*

    cl /nologo ^
      /LD /MTd ^
     /Fm!dllname!.map !WarningLevel! !IgnoreWarnings! !GenerateCompleteDebuggingInfo! !CompilationFlags! !IncludePaths! ..\!dllname!.cpp ^
      ..\game_memory.cpp ..\game_entity.cpp ..\Quaternion.cpp ..\game_render.cpp  ..\game_world.cpp  ..\game_assets.cpp ^
      /Fe:!dllnameOutput!.dll ^
      /link ^
      /DLL ^
      /incremental:no /opt:ref /PDB:!dllnameOutput!_!random!.pdb

)

popd

set BuildThis=Y
if "%BuildDllID%" NEQ "All" (
    if "%BuildDllID%" NEQ "4" (
        set BuildThis=N
    )
)
if "%BuildThis%"=="Y" (
    if !%Mode%!==!release!  (
        call python ..\shaders.py
    ) else (
        call python ..\shaders.py d
    )
)
