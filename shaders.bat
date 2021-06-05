@echo off
rem run within source folder src\
rem
set Mode=release
set OutputFolder=release
if !%1!==!debug! (
    set Mode=debug
    set OutputFolder=debug
)

if not exist shaders (
  mkdir shaders
)
set OutputFolderShaders=%OutputFolder%\shaders
if not exist %OutputFolderShaders% (
  mkdir %OutputFolderShaders%
)

set "ShaderValidator=C:\VulkanSDK\1.2.176.1\bin\glslangValidator.exe -V -o %OutputFolderShaders%\"
for /f %%f in ('dir /b .\shaders') do %ShaderValidator%%%f shaders\%%f
rem for /f %%f in ('dir /b .\shaders') do echo %ShaderValidator%%%f shaders\%%f

