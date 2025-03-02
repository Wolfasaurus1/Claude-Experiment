#pragma once

#include "Core/Application.hpp"
#include "Voxel/Chunk.hpp"
#include <memory>
#include <vector>
#include <string>

namespace VoxelEngine {

class VoxelGame : public Application {
public:
    VoxelGame();
    ~VoxelGame() override;

protected:
    // Application overrides
    void onInit() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onShutdown() override;
    
    // Input event handlers
    void onKeyPressed(int key) override;
    void onMouseMoved(double xPos, double yPos) override;
    void onWindowResized(int width, int height) override;
    
private:
    // Take a screenshot and save it to the screenshots directory
    void takeScreenshot();
    
    // Generate test world
    void generateTestWorld();
    
    // Set a voxel in the world
    void setVoxel(int x, int y, int z, VoxelType type);
    
    // Get a voxel from the world
    VoxelType getVoxel(int x, int y, int z) const;
    
    // Get the chunk at the given chunk coordinates
    Chunk* getChunk(int chunkX, int chunkY, int chunkZ) const;
    
    // Get or create the chunk at the given chunk coordinates
    Chunk* getOrCreateChunk(int chunkX, int chunkY, int chunkZ);
    
    // Convert world coordinates to chunk coordinates
    glm::ivec3 worldToChunkCoords(int x, int y, int z) const;
    
    // Convert world coordinates to local coordinates within a chunk
    glm::ivec3 worldToLocalCoords(int x, int y, int z) const;
    
    // Chunks
    std::vector<std::unique_ptr<Chunk>> m_Chunks;
    
    // Screenshot path
    std::string m_ScreenshotPath = "screenshots/";
    
    // Screenshot counter
    int m_ScreenshotCounter = 0;
    
    // Flag to take a screenshot on the next frame
    bool m_TakeScreenshotNextFrame = false;
};

} // namespace VoxelEngine 