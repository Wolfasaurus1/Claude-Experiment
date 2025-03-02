#pragma once

#include "VoxelRenderer.hpp"
#include <array>
#include <memory>
#include <glm/glm.hpp>

namespace VoxelEngine {

class Chunk {
public:
    static constexpr int CHUNK_SIZE_X = 16;
    static constexpr int CHUNK_SIZE_Y = 256;
    static constexpr int CHUNK_SIZE_Z = 16;
    
    Chunk(const glm::ivec3& position);
    ~Chunk();
    
    // Delete copy constructor and assignment operator
    Chunk(const Chunk&) = delete;
    Chunk& operator=(const Chunk&) = delete;
    
    // Set a voxel at the given local position
    void setVoxel(int x, int y, int z, VoxelType type);
    
    // Get a voxel at the given local position
    VoxelType getVoxel(int x, int y, int z) const;
    
    // Check if a position is valid within the chunk
    bool isValidPosition(int x, int y, int z) const;
    
    // Build the chunk mesh
    void buildMesh();
    
    // Render the chunk
    void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    
    // Get the chunk position
    const glm::ivec3& getPosition() const { return m_Position; }
    
    // Set neighboring chunks
    void setNeighbor(const glm::ivec3& direction, Chunk* chunk);
    
private:
    // Check if a face should be rendered (i.e., is the neighboring voxel transparent?)
    bool shouldRenderFace(int x, int y, int z, VoxelFace::Direction direction) const;
    
    // Helper method to check if a voxel type is transparent
    bool isVoxelTransparent(VoxelType type) const;
    
    // Get a voxel from neighboring chunks (if needed)
    VoxelType getVoxelWorldSpace(int x, int y, int z) const;
    
    using VoxelArray = std::array<VoxelType, CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z>;
    
    glm::ivec3 m_Position; // Chunk position in world space (in chunk coordinates)
    VoxelArray m_Voxels;   // Voxel data
    
    std::unique_ptr<VoxelRenderer> m_Renderer;
    
    // Neighboring chunks (for mesh generation)
    std::unordered_map<int, Chunk*> m_Neighbors; // Direction index -> Chunk
};

} // namespace VoxelEngine 