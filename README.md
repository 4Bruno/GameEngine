# GameEngine
Game engine using Vulkan and C with a subset of C++. Avoiding external libraries.
This is just for learning purposes.

# Design
* Limit use of external libraries
* Explicit Vulkan instantiation/usage
* Divide code between game/os/render
* Focus on simple C and plain data
* Game dll hot reload

# QA
* Why custom compilation?
I learnt how to use msvc clang compiler and only few dlls are created.
I like the simplicity/explicitness of a .bat file with 70 lines to handle compilation.
For debugging I use .clangcomplete and in vim I have shortcuts to build which takes
less time than switching windows.

* Compilation of shaders?
As for now using validator.exe. Everything hardcoded. 


# Dependencies
* For now using Quaternion external lib to test my matrix math

# How to build
```
git clone https://github.com/4Bruno/GameEngine.git
cd GameEngine
cd src
..\debug_build.bat
cd debug
win32_platform.exe
```
