#pragma once

#include "Core/Application.hpp"
#include "Voxel/Chunk.hpp"
#include "Renderer/Camera.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace VoxelEngine {

class VoxelGame : public Application {
public:
    VoxelGame();
    ~VoxelGame() override;

    void init(int windowWidth, int windowHeight, const std::string& title);
    void run();

protected:
    // Application overrides
    void onInit() override;
    void onUpdate(float deltaTime) override;
    void onRender() override;
    void onImGuiRender() override;
    void onShutdown() override;
    
    // Input event handlers
    void onKeyPressed(int key) override;
    void onMouseMoved(double xPos, double yPos) override;
    void onWindowResized(int width, int height) override;
    
private:
    void update();
    void render();
    void handleInput();
    void takeScreenshot();
    
    void generateTestWorld();
    
    // Helper methods for terrain generation
    void addTrees(Chunk* chunk, int count, int minHeight, int maxHeight);
    void addHouse(Chunk* chunk);
    void addTower(Chunk* chunk, bool isMountain);
    void addTemple(Chunk* chunk);
    
    // Convert 3D chunk coordinates to a unique key for the map
    uint64_t getChunkKey(int x, int y, int z) const;
    
    // Get or create a chunk at the specified coordinates
    Chunk* getOrCreateChunk(int x, int y, int z);
    
    // Get a chunk at the specified coordinates (returns nullptr if not found)
    Chunk* getChunk(int x, int y, int z) const;
    
    // Set a voxel at the specified world coordinates
    void setVoxel(int x, int y, int z, VoxelType type);
    
    // Get a voxel at the specified world coordinates
    VoxelType getVoxel(int x, int y, int z) const;
    
    // Convert world coordinates to chunk coordinates
    glm::ivec3 worldToChunkCoords(int x, int y, int z) const;
    
    // Convert world coordinates to local coordinates within a chunk
    glm::ivec3 worldToLocalCoords(int x, int y, int z) const;
    
    // Screenshot path
    std::string m_ScreenshotPath = "screenshots/";
    
    // Screenshot counter
    int m_ScreenshotCounter = 0;
    
    // Flag to take a screenshot on the next frame
    bool m_TakeScreenshotNextFrame = false;

    std::unordered_map<uint64_t, Chunk*> m_Chunks;
    Camera m_Camera;
    bool m_Running;
    
    int m_WindowWidth;
    int m_WindowHeight;
    
    float m_LastFrameTime;
    float m_DeltaTime;
    
    bool m_FirstMouse;
    float m_LastMouseX;
    float m_LastMouseY;
    
    bool m_WireframeMode;
};

} // namespace VoxelEngine 