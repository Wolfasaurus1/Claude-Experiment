#include "VoxelGame.hpp"
#include "Renderer/Camera.hpp"
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
    : Application("Voxel Game with Greedy Meshing", 1600, 900),
      m_Camera(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f),
      m_ScreenshotCounter(0),
      m_TakeScreenshotNextFrame(false)
{
    // Create screenshots directory if it doesn't exist
    m_ScreenshotPath = "screenshots/";
    std::filesystem::create_directory(m_ScreenshotPath);
}

VoxelGame::~VoxelGame() {
    // Clean up chunks
    for (auto& pair : m_Chunks) {
        delete pair.second;
    }
    m_Chunks.clear();
}

void VoxelGame::onInit() {
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Set clear color to sky blue
    glClearColor(0.5f, 0.7f, 1.0f, 1.0f);
    
    // Set initial camera position and rotation
    m_Camera.setPosition(glm::vec3(32.0f, 32.0f, 32.0f));
    m_Camera.setRotation(-45.0f, -30.0f);
    
    // Generate test world
    generateTestWorld();
}

void VoxelGame::onUpdate(float deltaTime) {
    // Handle camera movement
    // Forward/backward
    if (isKeyPressed(GLFW_KEY_W)) {
        m_Camera.moveForward(10.0f * deltaTime);
    }
    if (isKeyPressed(GLFW_KEY_S)) {
        m_Camera.moveForward(-10.0f * deltaTime);
    }
    
    // Left/right
    if (isKeyPressed(GLFW_KEY_A)) {
        m_Camera.moveRight(-10.0f * deltaTime);
    }
    if (isKeyPressed(GLFW_KEY_D)) {
        m_Camera.moveRight(10.0f * deltaTime);
    }
    
    // Up/down
    if (isKeyPressed(GLFW_KEY_SPACE)) {
        m_Camera.moveUp(10.0f * deltaTime);
    }
    if (isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        m_Camera.moveUp(-10.0f * deltaTime);
    }
    
    // Update camera
    m_Camera.update(deltaTime);
    
    // Take screenshot if requested
    if (m_TakeScreenshotNextFrame) {
        takeScreenshot();
        m_TakeScreenshotNextFrame = false;
    }
    
    // Toggle screenshot on F2
    if (isKeyPressed(GLFW_KEY_F2) && !isKeyPressedLastFrame(GLFW_KEY_F2)) {
        m_TakeScreenshotNextFrame = true;
    }
}

void VoxelGame::onRender() {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render all chunks
    for (const auto& pair : m_Chunks) {
        pair.second->render(m_Camera.getViewMatrix(), m_Camera.getProjectionMatrix());
    }
}

void VoxelGame::onImGuiRender() {
    // ImGui rendering if needed
}

void VoxelGame::onShutdown() {
    // Any additional cleanup
}

void VoxelGame::takeScreenshot() {
    // Create a unique filename with date and time
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << m_ScreenshotPath << "screenshot_" 
       << std::put_time(std::localtime(&in_time_t), "%Y%m%d_%H%M%S")
       << "_" << m_ScreenshotCounter << ".bmp";
    
    std::string filename = ss.str();
    m_ScreenshotCounter++;
    
    // Save the screenshot
    saveScreenshot(filename);
    
    std::cout << "Screenshot saved to " << filename << std::endl;
}

void VoxelGame::generateTestWorld() {
    // Generate a varied terrain with hills, valleys, and different biomes
    const int worldSizeX = 6; // in chunks (increased from 4)
    const int worldSizeZ = 6; // in chunks (increased from 4)
    
    // Simple noise function to generate terrain height
    auto simpleNoise = [](float x, float z) -> float {
        return sinf(x * 0.1f) * cosf(z * 0.1f) * 3.0f + 
               sinf(x * 0.05f + z * 0.05f) * 5.0f + 
               cosf(x * 0.02f - z * 0.03f) * 2.0f;
    };
    
    // Create chunks
    for (int cz = 0; cz < worldSizeZ; cz++) {
        for (int cx = 0; cx < worldSizeX; cx++) {
            // Create a new chunk
            Chunk* chunk = getOrCreateChunk(cx, 0, cz);
            
            // Define biome type based on position
            enum class BiomeType {
                Plains,
                Hills,
                Mountains,
                Desert,
                Forest
            };
            
            // Determine biome for this chunk
            BiomeType biome;
            float biomeNoise = simpleNoise(cx * 100.0f, cz * 100.0f);
            
            if (biomeNoise > 4.0f) {
                biome = BiomeType::Mountains;
            } else if (biomeNoise > 2.0f) {
                biome = BiomeType::Hills;
            } else if (biomeNoise > 0.0f) {
                biome = BiomeType::Plains;
            } else if (biomeNoise > -2.0f) {
                biome = BiomeType::Forest;
            } else {
                biome = BiomeType::Desert;
            }
            
            // Fill the chunk with terrain
            for (int x = 0; x < Chunk::CHUNK_SIZE_X; x++) {
                for (int z = 0; z < Chunk::CHUNK_SIZE_Z; z++) {
                    // Calculate world coordinates for noise
                    float worldX = cx * Chunk::CHUNK_SIZE_X + x;
                    float worldZ = cz * Chunk::CHUNK_SIZE_Z + z;
                    
                    // Base terrain height
                    int baseHeight = 4;
                    
                    // Apply height variation based on biome and noise
                    float terrainHeight = simpleNoise(worldX, worldZ);
                    
                    // Adjust height based on biome
                    switch (biome) {
                        case BiomeType::Mountains:
                            terrainHeight *= 2.5f;
                            baseHeight = 8;
                            break;
                        case BiomeType::Hills:
                            terrainHeight *= 1.5f;
                            baseHeight = 6;
                            break;
                        case BiomeType::Plains:
                            terrainHeight *= 0.5f;
                            baseHeight = 4;
                            break;
                        case BiomeType::Forest:
                            terrainHeight *= 0.8f;
                            baseHeight = 5;
                            break;
                        case BiomeType::Desert:
                            terrainHeight *= 0.3f;
                            terrainHeight = abs(terrainHeight); // Create dunes
                            baseHeight = 3;
                            break;
                    }
                    
                    // Calculate final height (clamped between 1 and 40)
                    int height = std::max(1, std::min(40, static_cast<int>(baseHeight + terrainHeight)));
                    
                    // Bedrock layer
                    chunk->setVoxel(x, 0, z, VoxelType::Stone);
                    
                    // Fill with stone up to near the top
                    for (int y = 1; y < height - 2; y++) {
                        chunk->setVoxel(x, y, z, VoxelType::Stone);
                    }
                    
                    // Add different top layers based on biome
                    switch (biome) {
                        case BiomeType::Desert:
                            // Desert has sand on top
                            chunk->setVoxel(x, height - 2, z, VoxelType::Sand);
                            chunk->setVoxel(x, height - 1, z, VoxelType::Sand);
                            break;
                        
                        case BiomeType::Mountains:
                            // Mountains have stone on top if high enough
                            if (height > 15) {
                                chunk->setVoxel(x, height - 2, z, VoxelType::Stone);
                                chunk->setVoxel(x, height - 1, z, VoxelType::Stone);
                            } else {
                                chunk->setVoxel(x, height - 2, z, VoxelType::Dirt);
                                chunk->setVoxel(x, height - 1, z, VoxelType::Grass);
                            }
                            break;
                        
                        default:
                            // Most biomes have dirt with grass on top
                            chunk->setVoxel(x, height - 2, z, VoxelType::Dirt);
                            chunk->setVoxel(x, height - 1, z, VoxelType::Grass);
                            break;
                    }
                    
                    // Add water at a fixed level
                    const int waterLevel = 5;
                    if (height < waterLevel) {
                        for (int y = height; y < waterLevel; y++) {
                            chunk->setVoxel(x, y, z, VoxelType::Water);
                        }
                        // Convert dirt/grass under water to sand
                        if (height >= 2) {
                            chunk->setVoxel(x, height - 1, z, VoxelType::Sand);
                        }
                    }
                }
            }
            
            // Add structures based on biome type
            switch (biome) {
                case BiomeType::Forest:
                    // Add many trees
                    addTrees(chunk, 15, 4, 7);
                    break;
                
                case BiomeType::Plains:
                    // Add few scattered trees
                    addTrees(chunk, 5, 3, 5);
                    break;
                
                case BiomeType::Hills:
                    // Add some trees and maybe a small structure
                    addTrees(chunk, 8, 4, 6);
                    
                    // Maybe add a small house on a hill
                    if ((cx + cz) % 3 == 0) {
                        addHouse(chunk);
                    }
                    break;
                
                case BiomeType::Mountains:
                    // Add a mountain tower
                    if ((cx + cz) % 4 == 0) {
                        addTower(chunk, true);
                    }
                    break;
                
                case BiomeType::Desert:
                    // Add a desert temple
                    if ((cx + cz) % 5 == 0) {
                        addTemple(chunk);
                    }
                    break;
            }
            
            // Build the chunk mesh
            chunk->buildMesh();
        }
    }
    
    // Set up chunk neighbors for proper face culling
    for (const auto& pair : m_Chunks) {
        Chunk* chunk = pair.second;
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
    for (const auto& pair : m_Chunks) {
        pair.second->buildMesh();
    }
}

// Helper method to add trees to a chunk
void VoxelGame::addTrees(Chunk* chunk, int count, int minHeight, int maxHeight) {
    // Get the chunk's position for world coordinates
    glm::ivec3 chunkPos = chunk->getPosition();
    
    // Add the specified number of trees
    for (int i = 0; i < count; i++) {
        // Random position within the chunk
        int x = rand() % Chunk::CHUNK_SIZE_X;
        int z = rand() % Chunk::CHUNK_SIZE_Z;
        
        // Find the surface height at this position
        int y = 0;
        for (int testY = Chunk::CHUNK_SIZE_Y - 1; testY >= 0; testY--) {
            VoxelType voxel = chunk->getVoxel(x, testY, z);
            if (voxel != VoxelType::Air && voxel != VoxelType::Water) {
                y = testY + 1;
                break;
            }
        }
        
        // Skip if underwater
        if (chunk->getVoxel(x, y, z) == VoxelType::Water) {
            continue;
        }
        
        // Random tree height within specified range
        int treeHeight = minHeight + rand() % (maxHeight - minHeight + 1);
        
        // Tree trunk
        for (int dy = 0; dy < treeHeight; dy++) {
            if (y + dy < Chunk::CHUNK_SIZE_Y) {
                chunk->setVoxel(x, y + dy, z, VoxelType::Wood);
            }
        }
        
        // Tree leaves
        for (int dy = treeHeight - 3; dy < treeHeight + 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                for (int dz = -2; dz <= 2; dz++) {
                    int lx = x + dx;
                    int ly = y + dy;
                    int lz = z + dz;
                    
                    // Skip if out of bounds
                    if (lx < 0 || lx >= Chunk::CHUNK_SIZE_X || 
                        ly < 0 || ly >= Chunk::CHUNK_SIZE_Y || 
                        lz < 0 || lz >= Chunk::CHUNK_SIZE_Z) {
                        continue;
                    }
                    
                    // Add leaves in a sphere-like pattern
                    float dist = sqrt(dx*dx + (dy-treeHeight+1)*(dy-treeHeight+1) + dz*dz);
                    if (dist <= 2.5f) {
                        // Skip the trunk
                        if (!(dx == 0 && dz == 0 && dy < treeHeight)) {
                            chunk->setVoxel(lx, ly, lz, VoxelType::Leaves);
                        }
                    }
                }
            }
        }
    }
}

// Helper method to add a house to a chunk
void VoxelGame::addHouse(Chunk* chunk) {
    // Find a suitable location for the house
    int centerX = Chunk::CHUNK_SIZE_X / 2;
    int centerZ = Chunk::CHUNK_SIZE_Z / 2;
    
    // Find the surface height at this position
    int baseY = 0;
    for (int y = Chunk::CHUNK_SIZE_Y - 1; y >= 0; y--) {
        VoxelType voxel = chunk->getVoxel(centerX, y, centerZ);
        if (voxel != VoxelType::Air && voxel != VoxelType::Water) {
            baseY = y + 1;
            break;
        }
    }
    
    // Skip if underwater
    if (chunk->getVoxel(centerX, baseY, centerZ) == VoxelType::Water) {
        return;
    }
    
    // House dimensions
    int width = 5;
    int depth = 6;
    int height = 4;
    
    // Build the house walls
    for (int y = baseY; y < baseY + height; y++) {
        for (int x = centerX - width/2; x <= centerX + width/2; x++) {
            for (int z = centerZ - depth/2; z <= centerZ + depth/2; z++) {
                // Skip if out of bounds
                if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                    y < 0 || y >= Chunk::CHUNK_SIZE_Y || 
                    z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                    continue;
                }
                
                // Only build the walls (hollow inside)
                if (x == centerX - width/2 || x == centerX + width/2 || 
                    z == centerZ - depth/2 || z == centerZ + depth/2) {
                    chunk->setVoxel(x, y, z, VoxelType::Wood);
                }
            }
        }
    }
    
    // Add a door
    int doorX = centerX;
    int doorZ = centerZ - depth/2;
    chunk->setVoxel(doorX, baseY, doorZ, VoxelType::Air);
    chunk->setVoxel(doorX, baseY + 1, doorZ, VoxelType::Air);
    
    // Add a roof (pyramid style)
    for (int layer = 0; layer <= width/2 + 1; layer++) {
        for (int x = centerX - width/2 + layer; x <= centerX + width/2 - layer; x++) {
            for (int z = centerZ - depth/2 + layer; z <= centerZ + depth/2 - layer; z++) {
                // Skip if out of bounds
                if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                    baseY + height + layer >= Chunk::CHUNK_SIZE_Y || 
                    z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                    continue;
                }
                
                chunk->setVoxel(x, baseY + height + layer, z, VoxelType::Wood);
            }
        }
    }
}

// Helper method to add a tower to a chunk
void VoxelGame::addTower(Chunk* chunk, bool isMountain) {
    // Find a suitable location for the tower
    int centerX = Chunk::CHUNK_SIZE_X / 2;
    int centerZ = Chunk::CHUNK_SIZE_Z / 2;
    
    // Find the surface height at this position
    int baseY = 0;
    for (int y = Chunk::CHUNK_SIZE_Y - 1; y >= 0; y--) {
        VoxelType voxel = chunk->getVoxel(centerX, y, centerZ);
        if (voxel != VoxelType::Air && voxel != VoxelType::Water) {
            baseY = y + 1;
            break;
        }
    }
    
    // Skip if underwater
    if (chunk->getVoxel(centerX, baseY, centerZ) == VoxelType::Water) {
        return;
    }
    
    // Tower dimensions
    int width = 5;
    int towerHeight = isMountain ? 12 : 8; // Taller on mountains
    
    // Build the tower
    for (int y = baseY; y < baseY + towerHeight; y++) {
        for (int x = centerX - width/2; x <= centerX + width/2; x++) {
            for (int z = centerZ - width/2; z <= centerZ + width/2; z++) {
                // Skip if out of bounds
                if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                    y < 0 || y >= Chunk::CHUNK_SIZE_Y || 
                    z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                    continue;
                }
                
                // Only build the walls (hollow inside)
                if (x == centerX - width/2 || x == centerX + width/2 || 
                    z == centerZ - width/2 || z == centerZ + width/2) {
                    
                    // Alternate stone and different materials for more visual interest
                    if (y % 2 == 0) {
                        chunk->setVoxel(x, y, z, VoxelType::Stone);
                    } else {
                        chunk->setVoxel(x, y, z, VoxelType::Sand);
                    }
                    
                    // Add windows
                    if (y % 3 == 0 && y > baseY + 1 && 
                        ((x == centerX && (z == centerZ - width/2 || z == centerZ + width/2)) ||
                         (z == centerZ && (x == centerX - width/2 || x == centerX + width/2)))) {
                        chunk->setVoxel(x, y, z, VoxelType::Air);
                    }
                }
            }
        }
    }
    
    // Add battlements on top
    for (int x = centerX - width/2; x <= centerX + width/2; x++) {
        for (int z = centerZ - width/2; z <= centerZ + width/2; z++) {
            // Skip if out of bounds
            if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                baseY + towerHeight >= Chunk::CHUNK_SIZE_Y || 
                z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                continue;
            }
            
            // Only on the edges
            if (x == centerX - width/2 || x == centerX + width/2 || 
                z == centerZ - width/2 || z == centerZ + width/2) {
                
                // Alternating pattern for battlements
                if ((x + z) % 2 == 0) {
                    chunk->setVoxel(x, baseY + towerHeight, z, VoxelType::Stone);
                }
            }
        }
    }
}

// Helper method to add a desert temple to a chunk
void VoxelGame::addTemple(Chunk* chunk) {
    // Find a suitable location for the temple
    int centerX = Chunk::CHUNK_SIZE_X / 2;
    int centerZ = Chunk::CHUNK_SIZE_Z / 2;
    
    // Find the surface height at this position
    int baseY = 0;
    for (int y = Chunk::CHUNK_SIZE_Y - 1; y >= 0; y--) {
        VoxelType voxel = chunk->getVoxel(centerX, y, centerZ);
        if (voxel != VoxelType::Air && voxel != VoxelType::Water) {
            baseY = y + 1;
            break;
        }
    }
    
    // Skip if underwater
    if (chunk->getVoxel(centerX, baseY, centerZ) == VoxelType::Water) {
        return;
    }
    
    // Temple dimensions
    int width = 9;
    int height = 6;
    
    // Build the base platform
    for (int y = baseY; y < baseY + 1; y++) {
        for (int x = centerX - width/2; x <= centerX + width/2; x++) {
            for (int z = centerZ - width/2; z <= centerZ + width/2; z++) {
                // Skip if out of bounds
                if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                    y < 0 || y >= Chunk::CHUNK_SIZE_Y || 
                    z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                    continue;
                }
                
                chunk->setVoxel(x, y, z, VoxelType::Sand);
            }
        }
    }
    
    // Build pyramid layers
    for (int layer = 0; layer < height; layer++) {
        for (int x = centerX - width/2 + layer; x <= centerX + width/2 - layer; x++) {
            for (int z = centerZ - width/2 + layer; z <= centerZ + width/2 - layer; z++) {
                // Skip if out of bounds
                if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                    baseY + 1 + layer >= Chunk::CHUNK_SIZE_Y || 
                    z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                    continue;
                }
                
                // Only build the outer layer
                if (layer == height - 1 || x == centerX - width/2 + layer || x == centerX + width/2 - layer || 
                    z == centerZ - width/2 + layer || z == centerZ + width/2 - layer) {
                    chunk->setVoxel(x, baseY + 1 + layer, z, VoxelType::Sand);
                }
            }
        }
    }
    
    // Add an entrance
    int entranceWidth = 2;
    for (int x = centerX - entranceWidth/2; x <= centerX + entranceWidth/2; x++) {
        for (int y = baseY + 1; y < baseY + 4; y++) {
            // Skip if out of bounds
            if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                y >= Chunk::CHUNK_SIZE_Y || 
                centerZ - width/2 < 0 || centerZ - width/2 >= Chunk::CHUNK_SIZE_Z) {
                continue;
            }
            
            chunk->setVoxel(x, y, centerZ - width/2, VoxelType::Air);
        }
    }
}

uint64_t VoxelGame::getChunkKey(int x, int y, int z) const {
    // Create a unique key from the chunk coordinates
    // This allows for a 21-bit range for each coordinate (enough for -1,048,576 to 1,048,575)
    return ((uint64_t)(x & 0x1FFFFF)) | 
           ((uint64_t)(y & 0x1FFFFF) << 21) | 
           ((uint64_t)(z & 0x1FFFFF) << 42);
}

Chunk* VoxelGame::getChunk(int chunkX, int chunkY, int chunkZ) const {
    uint64_t key = getChunkKey(chunkX, chunkY, chunkZ);
    auto it = m_Chunks.find(key);
    if (it != m_Chunks.end()) {
        return it->second;
    }
    return nullptr;
}

Chunk* VoxelGame::getOrCreateChunk(int chunkX, int chunkY, int chunkZ) {
    uint64_t key = getChunkKey(chunkX, chunkY, chunkZ);
    
    // Check if chunk already exists
    auto it = m_Chunks.find(key);
    if (it != m_Chunks.end()) {
        return it->second;
    }
    
    // Create a new chunk
    Chunk* chunk = new Chunk(glm::ivec3(chunkX, chunkY, chunkZ));
    m_Chunks[key] = chunk;
    
    return chunk;
}

void VoxelGame::setVoxel(int x, int y, int z, VoxelType type) {
    // Convert world coordinates to chunk coordinates
    glm::ivec3 chunkCoords = worldToChunkCoords(x, y, z);
    
    // Get or create the chunk
    Chunk* chunk = getOrCreateChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z);
    
    // Convert world coordinates to local coordinates
    glm::ivec3 localCoords = worldToLocalCoords(x, y, z);
    
    // Set the voxel in the chunk
    chunk->setVoxel(localCoords.x, localCoords.y, localCoords.z, type);
}

VoxelType VoxelGame::getVoxel(int x, int y, int z) const {
    // Convert world coordinates to chunk coordinates
    glm::ivec3 chunkCoords = worldToChunkCoords(x, y, z);
    
    // Get the chunk
    Chunk* chunk = getChunk(chunkCoords.x, chunkCoords.y, chunkCoords.z);
    if (!chunk) {
        return VoxelType::Air;
    }
    
    // Convert world coordinates to local coordinates
    glm::ivec3 localCoords = worldToLocalCoords(x, y, z);
    
    // Get the voxel from the chunk
    return chunk->getVoxel(localCoords.x, localCoords.y, localCoords.z);
}

glm::ivec3 VoxelGame::worldToChunkCoords(int x, int y, int z) const {
    // Convert world coordinates to chunk coordinates
    // Uses integer division to floor the values
    int chunkX = x >= 0 ? x / Chunk::CHUNK_SIZE_X : (x - Chunk::CHUNK_SIZE_X + 1) / Chunk::CHUNK_SIZE_X;
    int chunkY = y >= 0 ? y / Chunk::CHUNK_SIZE_Y : (y - Chunk::CHUNK_SIZE_Y + 1) / Chunk::CHUNK_SIZE_Y;
    int chunkZ = z >= 0 ? z / Chunk::CHUNK_SIZE_Z : (z - Chunk::CHUNK_SIZE_Z + 1) / Chunk::CHUNK_SIZE_Z;
    
    return glm::ivec3(chunkX, chunkY, chunkZ);
}

glm::ivec3 VoxelGame::worldToLocalCoords(int x, int y, int z) const {
    // Convert world coordinates to local coordinates within a chunk
    // Uses modulo to get the local offset
    int localX = x >= 0 ? x % Chunk::CHUNK_SIZE_X : (x % Chunk::CHUNK_SIZE_X + Chunk::CHUNK_SIZE_X) % Chunk::CHUNK_SIZE_X;
    int localY = y >= 0 ? y % Chunk::CHUNK_SIZE_Y : (y % Chunk::CHUNK_SIZE_Y + Chunk::CHUNK_SIZE_Y) % Chunk::CHUNK_SIZE_Y;
    int localZ = z >= 0 ? z % Chunk::CHUNK_SIZE_Z : (z % Chunk::CHUNK_SIZE_Z + Chunk::CHUNK_SIZE_Z) % Chunk::CHUNK_SIZE_Z;
    
    return glm::ivec3(localX, localY, localZ);
}

} // namespace VoxelEngine 