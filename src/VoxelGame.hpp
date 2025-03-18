#pragma once

#include "Core/Application.hpp"
#include "Voxel/Chunk.hpp"
#include "Renderer/Camera.hpp"
#include "Renderer/ShadowMap.hpp"
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
    void onWindowResized(int width, int height) override;
    
private:
    void update();
    void render();
    void handleInput();
    void takeScreenshot();
    void updateLightDirection(float deltaTime);
    
    // Shadow mapping methods
    void renderShadowPass();
    void renderSceneWithShadows();
    void updateLightSpaceMatrix();
    void calculateSceneBounds();
    
    void generateTestWorld();
    
    // Helper methods for terrain generation
    void addSkyscraper(Chunk* chunk);
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
    bool m_Running = true;
    
    int m_WindowWidth = 0;
    int m_WindowHeight = 0;
    
    float m_LastFrameTime = 0.0f;
    float m_DeltaTime = 0.0f;
    
    bool m_FirstMouse = true;
    float m_LastMouseX = 0.0f;
    float m_LastMouseY = 0.0f;
    
    bool m_WireframeMode = false;
    
    // Light properties for shadow mapping
    glm::vec3 m_LightDir = glm::normalize(glm::vec3(0.2f, -0.9f, 0.3f));
    float m_DayNightCycle = 0.0f; // 0.0 to 1.0, representing time of day
    float m_DayNightSpeed = 0.05f; // Speed of day/night cycle
    bool m_DayNightEnabled = true; // Enable or disable day/night cycle
    
    // Shadow mapping
    std::unique_ptr<ShadowMap> m_ShadowMap;
    bool m_ShadowsEnabled = true;
    glm::vec3 m_SceneCenter = glm::vec3(0.0f);
    float m_SceneRadius = 500.0f;
    
    // Player movement speed (units per second)
    float m_MovementSpeed = 30.0f;
};

} // namespace VoxelEngine 