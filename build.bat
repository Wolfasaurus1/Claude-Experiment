@echo off
echo Building Voxel Engine...

if not exist build mkdir build
cd build

cmake ..
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b %ERRORLEVEL%
)

cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Build successful!
echo Running application...
bin\Release\VoxelEngine.exe

cd ..
pause 