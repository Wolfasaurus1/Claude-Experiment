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
#include <cstdlib> // For rand()

namespace VoxelEngine {

VoxelGame::VoxelGame()
    : Application("Voxel Game with Greedy Meshing", 3840, 2160),
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
    glClearColor(0.741,0.847,1, 1.0f);
    
    // Set initial camera position and rotation
    m_Camera.setPosition(glm::vec3(32.0f, 32.0f, 32.0f));
    m_Camera.setRotation(-45.0f, -30.0f);
    
    // Hide cursor for better first-person navigation
    m_Window->disableCursor();
    
    // Generate test world
    generateTestWorld();

    m_FirstMouse = true;
}

void VoxelGame::onUpdate(float deltaTime) {
    // Handle camera movement
    // Forward/backward
    if (m_Window->isKeyPressed(GLFW_KEY_W)) {
        m_Camera.moveForward(m_MovementSpeed * deltaTime);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_S)) {
        m_Camera.moveForward(-m_MovementSpeed * deltaTime);
    }
    
    // Left/right
    if (m_Window->isKeyPressed(GLFW_KEY_A)) {
        m_Camera.moveRight(-m_MovementSpeed * deltaTime);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_D)) {
        m_Camera.moveRight(m_MovementSpeed * deltaTime);
    }
    
    // Up/down
    if (m_Window->isKeyPressed(GLFW_KEY_SPACE)) {
        m_Camera.moveUp(m_MovementSpeed * deltaTime);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        m_Camera.moveUp(-m_MovementSpeed * deltaTime);
    }
    
    // Adjust movement speed
    static bool wasPlusPressed = false;
    static bool wasMinusPressed = false;
    
    bool isPlusPressed = m_Window->isKeyPressed(GLFW_KEY_EQUAL); // + key (usually SHIFT+=)
    bool isMinusPressed = m_Window->isKeyPressed(GLFW_KEY_MINUS);
    
    // Increase speed with + key
    if (isPlusPressed && !wasPlusPressed) {
        m_MovementSpeed += 10.0f;
        std::cout << "Movement speed: " << m_MovementSpeed << std::endl;
    }
    
    // Decrease speed with - key (minimum 10.0f)
    if (isMinusPressed && !wasMinusPressed) {
        m_MovementSpeed = std::max(10.0f, m_MovementSpeed - 10.0f);
        std::cout << "Movement speed: " << m_MovementSpeed << std::endl;
    }
    
    wasPlusPressed = isPlusPressed;
    wasMinusPressed = isMinusPressed;
    
    // Process mouse input for camera look
    double xpos, ypos;
    m_Window->getCursorPosition(xpos, ypos);
    
    if (m_FirstMouse) {
        m_LastMouseX = xpos;
        m_LastMouseY = ypos;
        m_FirstMouse = false;
    }
    
    double xoffset = xpos - m_LastMouseX;
    double yoffset = m_LastMouseY - ypos; // Reversed since y-coordinates go from bottom to top
    
    m_LastMouseX = xpos;
    m_LastMouseY = ypos;
    
    // Rotate camera based on mouse movement
    m_Camera.processMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));

    // Update camera
    m_Camera.update(deltaTime);
    
    // Take screenshot if requested
    if (m_TakeScreenshotNextFrame) {
        takeScreenshot();
        m_TakeScreenshotNextFrame = false;
    }
    
    // Toggle screenshot on F2
    static bool wasF2Pressed = false;
    bool isF2Pressed = m_Window->isKeyPressed(GLFW_KEY_F2);
    if (isF2Pressed && !wasF2Pressed) {
        m_TakeScreenshotNextFrame = true;
    }
    wasF2Pressed = isF2Pressed;
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
    Screenshot::capture(filename);
    
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
                    int height = 4;
                 
                    // Bedrock layer
                    chunk->setVoxel(x, 0, z, VoxelType::Stone);
                    
                    // Fill with stone up to near the top
                    for (int y = 1; y < height - 2; y++) {
                        chunk->setVoxel(x, y, z, VoxelType::Stone);
                    }
                    
                    // Most biomes have dirt with grass on top
                    if(x == 0 || x == Chunk::CHUNK_SIZE_X - 1 || z == 0 || z == Chunk::CHUNK_SIZE_Z - 1){   
                        chunk->setVoxel(x, height - 1, z, VoxelType::Stone);
                    }
                    else{
                        chunk->setVoxel(x, height - 2, z, VoxelType::Dirt);
                        chunk->setVoxel(x, height - 1, z, VoxelType::Grass);
                    }
                }
            }
            
            // add skyscraper
            addSkyscraper(chunk);
            
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

void VoxelGame::addSkyscraper(Chunk* chunk) {
    // Find a suitable location for the skyscraper (center of the chunk)
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

    // Skyscraper base dimensions (randomized)
    int baseWidth = 6 + (rand() % 5); // Width between 6 and 10
    int baseDepth = 6 + (rand() % 5); // Depth between 6 and 10
    int totalHeight = 20 + (rand() % 31); // Height between 20 and 40
    int sectionHeight = 5 + (rand() % 3); // Each section is 5-7 voxels tall

    int currentY = baseY;

    // Build the skyscraper in sections
    int currentWidth = baseWidth;
    int currentDepth = baseDepth;
    while (currentY < baseY + totalHeight && currentY < Chunk::CHUNK_SIZE_Y) {
        // Build one section
        for (int y = currentY; y < currentY + sectionHeight && y < baseY + totalHeight; y++) {
            for (int x = centerX - currentWidth / 2; x <= centerX + currentWidth / 2; x++) {
                for (int z = centerZ - currentDepth / 2; z <= centerZ + currentDepth / 2; z++) {
                    // Skip if out of bounds
                    if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                        y >= Chunk::CHUNK_SIZE_Y || 
                        z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                        continue;
                    }

                    // Build only the walls (hollow inside)
                    if (x == centerX - currentWidth / 2 || x == centerX + currentWidth / 2 ||
                        z == centerZ - currentDepth / 2 || z == centerZ + currentDepth / 2) {
                        chunk->setVoxel(x, y, z, VoxelType::Stone); // Use Wood or another type
                    }
                }
            }
        }

        // Move up to the next section
        currentY += sectionHeight;

        // Reduce width and depth for the next section (setback effect), but not below 4
        currentWidth = std::max(4, currentWidth - (rand() % 3)); // Shrink by 0-2
        currentDepth = std::max(4, currentDepth - (rand() % 3));
    }

    //currentY -= sectionHeight;

    // Add a simple flat roof or spire
    if (currentY < Chunk::CHUNK_SIZE_Y) {
        for (int x = centerX - currentWidth / 2; x <= centerX + currentWidth / 2; x++) {
            for (int z = centerZ - currentDepth / 2; z <= centerZ + currentDepth / 2; z++) {
                if (x < 0 || x >= Chunk::CHUNK_SIZE_X || 
                    currentY >= Chunk::CHUNK_SIZE_Y || 
                    z < 0 || z >= Chunk::CHUNK_SIZE_Z) {
                    continue;
                }
                chunk->setVoxel(x, baseY + totalHeight, z, VoxelType::Wood); // Flat roof
            }
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

void VoxelGame::onKeyPressed(int key) {
    // Handle key press events
    if (key == GLFW_KEY_F2) {
        m_TakeScreenshotNextFrame = true;
    }
}

void VoxelGame::onWindowResized(int width, int height) {
    // Update viewport and camera aspect ratio
    m_WindowWidth = width;
    m_WindowHeight = height;
    
    // Update camera projection matrix
    m_Camera.setProjectionMatrix(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);
    
    // Update OpenGL viewport
    glViewport(0, 0, width, height);
}

void VoxelGame::init(int windowWidth, int windowHeight, const std::string& title) {
    // Store window dimensions
    m_WindowWidth = windowWidth;
    m_WindowHeight = windowHeight;
    
    // Initialize member variables
    m_Running = true;
    m_LastFrameTime = 0.0f;
    m_DeltaTime = 0.0f;
    m_FirstMouse = true;
    m_LastMouseX = windowWidth / 2.0f;
    m_LastMouseY = windowHeight / 2.0f;
    m_WireframeMode = false;
    
    // Create screenshots directory if it doesn't exist
    std::filesystem::create_directory(m_ScreenshotPath);
}

void VoxelGame::run() {
    // Initialize the game
    init(1600, 900, "Voxel Game with Greedy Meshing");
    
    // Run the application
    Application::run();
}

} // namespace VoxelEngine 