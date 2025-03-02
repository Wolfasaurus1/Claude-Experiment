#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>

namespace VoxelEngine {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
    glm::vec4 color;
};

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

    // Delete copy constructor and assignment operator
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    void render() const;
    
    // Getters
    unsigned int getVAO() const { return m_VAO; }
    unsigned int getVertexCount() const { return m_VertexCount; }
    unsigned int getIndexCount() const { return m_IndexCount; }

    // Static mesh creation helpers
    static Mesh* createCube(float size = 1.0f, const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    static Mesh* createPlane(float size = 1.0f, const glm::vec4& color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

private:
    void setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    
    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
    
    unsigned int m_VertexCount = 0;
    unsigned int m_IndexCount = 0;
};

} // namespace VoxelEngine 