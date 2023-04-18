# Project2 Server

# Requirements
- C++20
- Windows
- [CMake](https://cmake.org/)
- [Visual Studio 2022 (MSVC 17)](https://visualstudio.microsoft.com/#vs-section)

    Be sure that the project is set to compile using C++20 features. This can be set by opening the `Build/Project2_Server.sln` in Visual Studio, going to `Project>Project2_Server Properties>Configuration Properties>General>C++ Language Standard` and setting it to `ISO C++20 Standard (/std:c++20)`

# Build
Run  the following:
```
mkdir Build
cd Build
cmake ..
cmake --build .
```
This creates `Build/Project2_Server.sln` for use with Visual Studio.

The executable can be found at `Build/Debug/Project2_Server.exe`
