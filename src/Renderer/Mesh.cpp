#include "Mesh.hpp"

namespace VoxelEngine {

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    setupMesh(vertices, indices);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void Mesh::setupMesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    m_VertexCount = static_cast<unsigned int>(vertices.size());
    m_IndexCount = static_cast<unsigned int>(indices.size());

    // Create buffers/arrays
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    
    // Load data into vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    // Load data into element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    
    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    
    // Vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));
    
    // Vertex Colors (now vec4 with alpha)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);
}

void Mesh::render() const {
    // Draw mesh
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Mesh* Mesh::createCube(float size, const glm::vec4& color) {
    float halfSize = size / 2.0f;
    
    // Define vertices
    std::vector<Vertex> vertices = {
        // Front face
        { glm::vec3(-halfSize, -halfSize,  halfSize), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f), color }, // Bottom-left
        { glm::vec3( halfSize, -halfSize,  halfSize), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f), color }, // Bottom-right
        { glm::vec3( halfSize,  halfSize,  halfSize), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f), color }, // Top-right
        { glm::vec3(-halfSize,  halfSize,  halfSize), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f), color }, // Top-left
        
        // Back face
        { glm::vec3(-halfSize, -halfSize, -halfSize), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f), color }, // Bottom-left
        { glm::vec3(-halfSize,  halfSize, -halfSize), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f), color }, // Top-left
        { glm::vec3( halfSize,  halfSize, -halfSize), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f), color }, // Top-right
        { glm::vec3( halfSize, -halfSize, -halfSize), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f), color }, // Bottom-right
        
        // Top face
        { glm::vec3(-halfSize,  halfSize, -halfSize), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f), color }, // Back-left
        { glm::vec3(-halfSize,  halfSize,  halfSize), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), color }, // Front-left
        { glm::vec3( halfSize,  halfSize,  halfSize), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), color }, // Front-right
        { glm::vec3( halfSize,  halfSize, -halfSize), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f), color }, // Back-right
        
        // Bottom face
        { glm::vec3(-halfSize, -halfSize, -halfSize), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f), color }, // Back-left
        { glm::vec3( halfSize, -halfSize, -halfSize), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f), color }, // Back-right
        { glm::vec3( halfSize, -halfSize,  halfSize), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f), color }, // Front-right
        { glm::vec3(-halfSize, -halfSize,  halfSize), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f), color }, // Front-left
        
        // Right face
        { glm::vec3( halfSize, -halfSize, -halfSize), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f), color }, // Bottom-back
        { glm::vec3( halfSize,  halfSize, -halfSize), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), color }, // Top-back
        { glm::vec3( halfSize,  halfSize,  halfSize), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), color }, // Top-front
        { glm::vec3( halfSize, -halfSize,  halfSize), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), color }, // Bottom-front
        
        // Left face
        { glm::vec3(-halfSize, -halfSize, -halfSize), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f), color }, // Bottom-back
        { glm::vec3(-halfSize, -halfSize,  halfSize), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f), color }, // Bottom-front
        { glm::vec3(-halfSize,  halfSize,  halfSize), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f), color }, // Top-front
        { glm::vec3(-halfSize,  halfSize, -halfSize), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f), color }  // Top-back
    };

    // Define indices
    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0,         // Front face
        4, 5, 6, 6, 7, 4,         // Back face
        8, 9, 10, 10, 11, 8,      // Top face
        12, 13, 14, 14, 15, 12,   // Bottom face
        16, 17, 18, 18, 19, 16,   // Right face
        20, 21, 22, 22, 23, 20    // Left face
    };

    return new Mesh(vertices, indices);
}

Mesh* Mesh::createPlane(float size, const glm::vec4& color) {
    float halfSize = size / 2.0f;
    
    // Define vertices
    std::vector<Vertex> vertices = {
        { glm::vec3(-halfSize, 0.0f, -halfSize), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f), color }, // Back-left
        { glm::vec3( halfSize, 0.0f, -halfSize), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f), color }, // Back-right
        { glm::vec3( halfSize, 0.0f,  halfSize), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f), color }, // Front-right
        { glm::vec3(-halfSize, 0.0f,  halfSize), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f), color }  // Front-left
    };

    // Define indices
    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0  // Top face
    };

    return new Mesh(vertices, indices);
}

} // namespace VoxelEngine 