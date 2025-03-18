#pragma once

#include "../Renderer/Mesh.hpp"
#include "../Renderer/Shader.hpp"
#include "../Renderer/ShadowMap.hpp"
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
    
    // Render the voxel mesh with full lighting
    void render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
    
    // Render the mesh only without any lighting calculations (for shadow pass)
    void renderMeshOnly(Shader* shader);
    
    // Render the shadow pass
    void renderShadowPass();
    
    // Update light position or direction
    void updateLight(const glm::vec3& lightDir, const glm::vec3& center, float radius);
    
    // Set light properties
    void setLightColor(const glm::vec3& color) { m_LightColor = color; }
    void setLightDirection(const glm::vec3& direction) { m_LightDir = direction; }
    
    // Enable or disable shadows
    void enableShadows(bool enable) { m_ShadowsEnabled = enable; }
    bool areShadowsEnabled() const { return m_ShadowsEnabled; }
    
    // Set the light space matrix directly
    void setLightSpaceMatrix(const glm::mat4& matrix) { m_LightSpaceMatrix = matrix; }
    const glm::mat4& getLightSpaceMatrix() const { return m_LightSpaceMatrix; }
    
    // Get the mesh (if one exists)
    Mesh* getMesh() const { return m_Mesh.get(); }
    
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
    std::unique_ptr<ShadowMap> m_ShadowMap;
    
    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indices;
    std::vector<VoxelFace> m_Faces;
    std::vector<MergedFace> m_MergedFaces;
    
    // Light properties
    glm::vec3 m_LightDir = glm::normalize(glm::vec3(0.2f, -0.9f, 0.3f));
    glm::vec3 m_LightColor = glm::vec3(1.0f, 0.75f, 0.7f);
    
    // Shadow mapping
    bool m_ShadowsEnabled = true;
    glm::mat4 m_LightSpaceMatrix = glm::mat4(1.0f);
    
    // Cache for voxel colors
    std::unordered_map<VoxelType, glm::vec4> m_ColorCache;
    
    // Constants
    const float m_VoxelSize = 1.0f;
};

} // namespace VoxelEngine 