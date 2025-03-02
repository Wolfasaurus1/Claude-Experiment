#pragma once

#include "../Renderer/Mesh.hpp"
#include "../Renderer/Shader.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <array>
#include <functional>

namespace VoxelEngine {

enum class VoxelType {
    Air = 0,
    Grass,
    Dirt,
    Stone,
    Sand,
    Water,
    Wood,
    Leaves,
    // Add more voxel types as needed
};

struct VoxelFace {
    enum class Direction {
        Front,
        Back,
        Top,
        Bottom,
        Right,
        Left
    };
    
    Direction direction;
    VoxelType type;
    glm::ivec3 position;
};

// Structure to hold information about a merged face
struct MergedFace {
    VoxelFace::Direction direction;
    VoxelType type;
    glm::ivec3 start; // Starting position of the merged face
    glm::ivec2 size;  // Size of the merged face (width, height)
};

class VoxelRenderer {
public:
    VoxelRenderer();
    ~VoxelRenderer();
    
    // Delete copy constructor and assignment operator
    VoxelRenderer(const VoxelRenderer&) = delete;
    VoxelRenderer& operator=(const VoxelRenderer&) = delete;
    
    // Initialize the renderer
    void init();
    
    // Add a voxel face to be rendered
    void addFace(const VoxelFace& face);
    
    // Clear all voxel faces
    void clear();
    
    // Build the mesh from all added faces (naive approach)
    void buildMesh();
    
    // Build the mesh using greedy meshing algorithm
    void buildGreedyMesh(int chunkSizeX, int chunkSizeY, int chunkSizeZ, 
                         const std::function<VoxelType(int, int, int)>& getVoxelFunc,
                         const std::function<bool(int, int, int, VoxelFace::Direction)>& shouldRenderFaceFunc,
                         const glm::ivec3& chunkPos);
    
    // Render the voxel mesh
    void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    
private:
    // Get the color for a voxel type
    glm::vec4 getVoxelColor(VoxelType type) const;
    
    // Generate vertices for a voxel face
    std::vector<Vertex> generateFaceVertices(const VoxelFace& face) const;
    
    // Generate vertices for a merged face
    std::vector<Vertex> generateMergedFaceVertices(const MergedFace& face) const;
    
    // Generate indices for a voxel face
    std::vector<unsigned int> generateFaceIndices(unsigned int baseIndex) const;
    
    std::unique_ptr<Shader> m_Shader;
    std::unique_ptr<Mesh> m_Mesh;
    
    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indices;
    std::vector<VoxelFace> m_Faces;
    std::vector<MergedFace> m_MergedFaces;
    
    // Cache for voxel colors
    std::unordered_map<VoxelType, glm::vec4> m_ColorCache;
    
    // Constants
    const float m_VoxelSize = 1.0f;
};

} // namespace VoxelEngine 