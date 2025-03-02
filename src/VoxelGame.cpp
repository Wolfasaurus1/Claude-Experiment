#include "VoxelGame.hpp"
#include "Renderer/Screenshot.hpp"
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace VoxelEngine {

VoxelGame::VoxelGame()
    : Application("Voxel Game with Greedy Meshing", 1600, 900)
{
}

VoxelGame::~VoxelGame() {
    // Clean up resources specific to VoxelGame
    // The base class will handle its own cleanup
    m_Chunks.clear();
}

void VoxelGame::onInit() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Set clear color to sky blue
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    
    // Generate test world
    generateTestWorld();
    
    // Calculate the center of the world
    float worldCenterX = 4.0f * Chunk::CHUNK_SIZE_X * 0.5f;
    float worldCenterZ = 4.0f * Chunk::CHUNK_SIZE_Z * 0.5f;
    
    // Set up camera to better see the terrain
    m_Camera->setPosition(glm::vec3(worldCenterX + 20.0f, 30.0f, worldCenterZ + 20.0f));
    m_Camera->setRotation(-45.0f, -35.0f); // Look down at an angle toward the center
    
    // Disable cursor for camera control
    m_Window->disableCursor();
    
    // Take an initial screenshot after the first frame
    m_TakeScreenshotNextFrame = true;
}

void VoxelGame::onUpdate(float deltaTime) {
    // Process camera movement
    const float cameraSpeed = 5.0f * deltaTime;
    
    if (m_Window->isKeyPressed(GLFW_KEY_W)) {
        m_Camera->moveForward(cameraSpeed);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_S)) {
        m_Camera->moveForward(-cameraSpeed);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_A)) {
        m_Camera->moveRight(-cameraSpeed);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_D)) {
        m_Camera->moveRight(cameraSpeed);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_SPACE)) {
        m_Camera->moveUp(cameraSpeed);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        m_Camera->moveUp(-cameraSpeed);
    }
    
    // Update camera
    m_Camera->update(deltaTime);
}

void VoxelGame::onRender() {
    // Clear the screen with the sky color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set viewport
    int width = m_Window->getWidth();
    int height = m_Window->getHeight();
    glViewport(0, 0, width, height);
    
    // Render all chunks
    for (const auto& chunk : m_Chunks) {
        chunk->render(m_Camera->getViewMatrix(), m_Camera->getProjectionMatrix());
    }
    
    // Take a screenshot if the flag is set
    if (m_TakeScreenshotNextFrame) {
        m_TakeScreenshotNextFrame = false;
        // Wait for rendering to complete
        glFinish();
        // Take the screenshot
        takeScreenshot();
    }
}

void VoxelGame::onShutdown() {
    m_Chunks.clear();
}

void VoxelGame::onKeyPressed(int key) {
    if (key == GLFW_KEY_ESCAPE) {
        close();
    }
    else if (key == GLFW_KEY_F2 || key == GLFW_KEY_P) {
        // Set the flag to take a screenshot on the next frame
        m_TakeScreenshotNextFrame = true;
        std::cout << "Screenshot requested - will be taken on next frame" << std::endl;
    }
}

void VoxelGame::onMouseMoved(double xPos, double yPos) {
    static bool firstMouse = true;
    static double lastX = 0.0, lastY = 0.0;
    
    if (firstMouse) {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    
    // Calculate offset
    float xOffset = static_cast<float>(xPos - lastX);
    float yOffset = static_cast<float>(lastY - yPos); // Reversed: y ranges bottom to top
    
    // Update last position
    lastX = xPos;
    lastY = yPos;
    
    // Process mouse movement for camera rotation
    m_Camera->processMouseMovement(xOffset, yOffset);
}

void VoxelGame::onWindowResized(int width, int height) {
    // Update viewport and camera projection is handled in the Application base class
}

void VoxelGame::generateTestWorld() {
    // Generate a simple flat terrain with some structures
    const int worldSizeX = 4; // in chunks
    const int worldSizeZ = 4; // in chunks
    
    // Create chunks
    for (int cz = 0; cz < worldSizeZ; cz++) {
        for (int cx = 0; cx < worldSizeX; cx++) {
            // Create a new chunk
            Chunk* chunk = getOrCreateChunk(cx, 0, cz);
            
            // Fill the chunk with terrain
            for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
                for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
                    // Bedrock layer
                    chunk->setVoxel(x, 0, z, VoxelType::Stone);
                    
                    // Dirt layers
                    for (int y = 1; y < 4; y++) {
                        chunk->setVoxel(x, y, z, VoxelType::Dirt);
                    }
                    
                    // Grass top layer
                    chunk->setVoxel(x, 4, z, VoxelType::Grass);
                    
                    // Add some water features
                    if ((cx == 1 || cx == 2) && (cz == 1 || cz == 2) && 
                        x > 4 && x < 12 && z > 4 && z < 12) {
                        // Create a lake in the center of the world
                        chunk->setVoxel(x, 4, z, VoxelType::Water);
                        chunk->setVoxel(x, 3, z, VoxelType::Sand);
                        chunk->setVoxel(x, 2, z, VoxelType::Sand);
                    }
                    
                    // Add sand around the lake
                    if ((cx == 1 || cx == 2) && (cz == 1 || cz == 2) && 
                        ((x >= 3 && x <= 4 && z >= 3 && z <= 12) ||
                         (x >= 12 && x <= 13 && z >= 3 && z <= 12) ||
                         (x >= 3 && x <= 13 && z >= 3 && z <= 4) ||
                         (x >= 3 && x <= 13 && z >= 12 && z <= 13))) {
                        chunk->setVoxel(x, 4, z, VoxelType::Sand);
                    }
                }
            }
            
            // Add a simple structure in each chunk
            int centerX = Chunk::CHUNK_SIZE_X / 2;
            int centerZ = Chunk::CHUNK_SIZE_Z / 2;
            
            // Base height
            int baseY = 5;
            
            // Skip structures in the lake area
            bool isLakeArea = (cx == 1 || cx == 2) && (cz == 1 || cz == 2);
            
            if (!isLakeArea) {
                if ((cx % 2 == 0 && cz % 2 == 0) || (cx % 2 == 1 && cz % 2 == 1)) {
                    // Add a stone tower
                    for (int y = baseY; y < baseY + 6; y++) {
                        for (int x = centerX - 2; x <= centerX + 2; x++) {
                            for (int z = centerZ - 2; z <= centerZ + 2; z++) {
                                // Skip if out of bounds
                                if (x < 0 || x >= Chunk::CHUNK_SIZE_X || z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                                    continue;
                                }
                                
                                // Only build the walls (hollow inside)
                                if (x == centerX - 2 || x == centerX + 2 || z == centerZ - 2 || z == centerZ + 2) {
                                    // Alternate stone and different materials for more visual interest
                                    if (y % 2 == 0) {
                                        chunk->setVoxel(x, y, z, VoxelType::Stone);
                                    } else {
                                        chunk->setVoxel(x, y, z, VoxelType::Sand);
                                    }
                                }
                            }
                        }
                    }
                    
                    // Add a pyramid top
                    for (int layer = 0; layer < 3; layer++) {
                        int y = baseY + 6 + layer;
                        for (int x = centerX - 2 + layer; x <= centerX + 2 - layer; x++) {
                            for (int z = centerZ - 2 + layer; z <= centerZ + 2 - layer; z++) {
                                chunk->setVoxel(x, y, z, VoxelType::Stone);
                            }
                        }
                    }
                } else {
                    // Add trees
                    int treeHeight = 5;
                    
                    // Tree trunk
                    for (int y = baseY; y < baseY + treeHeight; y++) {
                        chunk->setVoxel(centerX, y, centerZ, VoxelType::Wood);
                    }
                    
                    // Tree leaves
                    for (int y = baseY + treeHeight - 2; y < baseY + treeHeight + 2; y++) {
                        for (int x = centerX - 2; x <= centerX + 2; x++) {
                            for (int z = centerZ - 2; z <= centerZ + 2; z++) {
                                // Skip if out of bounds
                                if (x < 0 || x >= Chunk::CHUNK_SIZE_X || z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                                    continue;
                                }
                                
                                // Add leaves in a sphere-like pattern
                                int dx = x - centerX;
                                int dy = y - (baseY + treeHeight);
                                int dz = z - centerZ;
                                
                                if (dx * dx + dy * dy + dz * dz <= 8) {
                                    // Skip the trunk
                                    if (!(x == centerX && z == centerZ && y < baseY + treeHeight)) {
                                        chunk->setVoxel(x, y, z, VoxelType::Leaves);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // Build the chunk mesh
            chunk->buildMesh();
        }
    }
    
    // Set up chunk neighbors for proper face culling
    for (const auto& chunk : m_Chunks) {
        glm::ivec3 pos = chunk->getPosition();
        
        // Set neighbors for all directions
        const glm::ivec3 directions[] = {
            glm::ivec3(1, 0, 0),   // Right
            glm::ivec3(-1, 0, 0),  // Left
            glm::ivec3(0, 1, 0),   // Top
            glm::ivec3(0, -1, 0),  // Bottom
            glm::ivec3(0, 0, 1),   // Front
            glm::ivec3(0, 0, -1)   // Back
        };
        
        for (const auto& dir : directions) {
            glm::ivec3 neighborPos = pos + dir;
            Chunk* neighbor = getChunk(neighborPos.x, neighborPos.y, neighborPos.z);
            if (neighbor) {
                chunk->setNeighbor(dir, neighbor);
            }
        }
    }
    
    // Rebuild all chunk meshes now that neighbors are set
    for (const auto& chunk : m_Chunks) {
        chunk->buildMesh();
    }
}

void VoxelGame::setVoxel(int x, int y, int z, VoxelType type) {
    glm::ivec3 chunkCoords = worldToChunkCoords(x, y, z);
    glm::ivec3 localCoords = worldToLocalCoords(x, y, z);
    
    Chunk* chunk = getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z);
    if (chunk) {
        chunk->setVoxel(localCoords.x, localCoords.y, localCoords.z, type);
    }
}

VoxelType VoxelGame::getVoxel(int x, int y, int z) const {
    glm::ivec3 chunkCoords = worldToChunkCoords(x, y, z);
    glm::ivec3 localCoords = worldToLocalCoords(x, y, z);
    
    Chunk* chunk = getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z);
    if (chunk) {
        return chunk->getVoxel(localCoords.x, localCoords.y, localCoords.z);
    }
    
    return VoxelType::Air;
}

Chunk* VoxelGame::getChunk(int chunkX, int chunkY, int chunkZ) const {
    for (const auto& chunk : m_Chunks) {
        glm::ivec3 pos = chunk->getPosition();
        if (pos.x == chunkX && pos.y == chunkY && pos.z == chunkZ) {
            return chunk.get();
        }
    }
    
    return nullptr;
}

Chunk* VoxelGame::getOrCreateChunk(int chunkX, int chunkY, int chunkZ) {
    // Try to find existing chunk
    Chunk* existingChunk = getChunk(chunkX, chunkY, chunkZ);
    if (existingChunk) {
        return existingChunk;
    }
    
    // Create new chunk
    glm::ivec3 chunkPos(chunkX, chunkY, chunkZ);
    auto chunk = std::make_unique<Chunk>(chunkPos);
    m_Chunks.push_back(std::move(chunk));
    
    return m_Chunks.back().get();
}

glm::ivec3 VoxelGame::worldToChunkCoords(int x, int y, int z) const {
    return glm::ivec3(
        std::floor(static_cast<float>(x) / Chunk::CHUNK_SIZE_X),
        std::floor(static_cast<float>(y) / Chunk::CHUNK_SIZE_Y),
        std::floor(static_cast<float>(z) / Chunk::CHUNK_SIZE_Z)
    );
}

glm::ivec3 VoxelGame::worldToLocalCoords(int x, int y, int z) const {
    // Use modulo to get local coordinates
    // We need to handle negative coordinates correctly
    int localX = ((x % Chunk::CHUNK_SIZE_X) + Chunk::CHUNK_SIZE_X) % Chunk::CHUNK_SIZE_X;
    int localY = ((y % Chunk::CHUNK_SIZE_Y) + Chunk::CHUNK_SIZE_Y) % Chunk::CHUNK_SIZE_Y;
    int localZ = ((z % Chunk::CHUNK_SIZE_Z) + Chunk::CHUNK_SIZE_Z) % Chunk::CHUNK_SIZE_Z;
    
    return glm::ivec3(localX, localY, localZ);
}

void VoxelGame::takeScreenshot() {
    // Use a path we know exists - current directory
    std::string screenshotDir = "./screenshots/";
    
    // Create screenshots directory if it doesn't exist
    std::filesystem::create_directories(screenshotDir);
    
    // Generate a filename with timestamp
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << screenshotDir << "screenshot_" 
       << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S") 
       << "_" << (m_ScreenshotCounter++) << ".bmp";
    
    std::string filename = ss.str();
    
    // Print current directory for debugging
    std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    std::cout << "Saving screenshot to: " << std::filesystem::absolute(filename) << std::endl;
    
    // Capture screenshot
    if (Screenshot::capture(filename)) {
        std::cout << "Screenshot saved: " << filename << std::endl;
    } else {
        std::cerr << "Failed to save screenshot" << std::endl;
    }
}

} // namespace VoxelEngine 