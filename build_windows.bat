@echo off
echo Building Linux Speed Meter for Windows...

REM Create build directory
if not exist build-windows mkdir build-windows
cd build-windows

REM Configure with CMake
cmake -G "Visual Studio 16 2019" -A x64 .. -DCMAKE_PREFIX_PATH="C:\Qt\6.5.0\msvc2019_64" -DCMAKE_BUILD_TYPE=Release

REM Build the project
cmake --build . --config Release

echo Build completed!
echo Executable should be in build-windows\Release\LinuxSpeedMeter.exe

pause