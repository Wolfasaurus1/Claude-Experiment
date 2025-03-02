#include "VoxelGame.hpp"
#include <iostream>

int main() {
    try {
        // Create and run the voxel game
        VoxelEngine::VoxelGame game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
} 