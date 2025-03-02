# Voxel Engine

A Minecraft-like voxel engine built with OpenGL.

## Features

- Modern OpenGL rendering with shaders
- Chunk-based voxel world
- Camera system for first-person navigation
- Basic terrain generation
- Different voxel types with color
- Face culling for improved performance
- Simple lighting system

## Prerequisites

- CMake (version 3.10 or higher)
- C++ compiler with C++17 support
- OpenGL compatible graphics card and drivers

## Building the Application

### Windows

1. Run the `build.bat` script, or:

2. Create a build directory:
```
mkdir build
cd build
```

3. Generate the build files:
```
cmake ..
```

4. Build the project:
```
cmake --build . --config Release
```

5. Run the application:
```
bin\Release\VoxelEngine.exe
```

### Linux/macOS

1. Create a build directory:
```
mkdir build
cd build
```

2. Generate the build files:
```
cmake ..
```

3. Build the project:
```
make
```

4. Run the application:
```
./bin/VoxelEngine
```

## Controls

- `W`, `A`, `S`, `D` - Move the camera
- `Space` - Move up
- `Left Shift` - Move down
- `Mouse` - Look around (right-click and move mouse)
- `ESC` - Close the application

## Project Structure

- `src/Core/` - Core application and window management
- `src/Renderer/` - Rendering classes (shaders, meshes, camera, etc.)
- `src/Voxel/` - Voxel-specific classes (chunks, voxel types, etc.)
- `Libraries/include/` - Header files for GLFW, GLAD, and GLM
- `Libraries/bin/` - Binary libraries (GLFW)

## Future Enhancements

- Texture support for different block types
- Procedural terrain generation with biomes
- Physics and collision detection
- Block placement and destruction
- Water simulation
- Dynamic lighting with day/night cycle
- Multiplayer support 