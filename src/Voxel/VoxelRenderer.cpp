#include "VoxelRenderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace VoxelEngine {

VoxelRenderer::VoxelRenderer() {
    init();
}

VoxelRenderer::~VoxelRenderer() {
    // Resources are automatically cleaned up by unique_ptr
}

void VoxelRenderer::init() {
    // Initialize shader
    const std::string vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;
        layout (location = 3) in vec4 aColor;

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;
        out vec4 Color;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoords = aTexCoords;
            Color = aColor;
            
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const std::string fragmentShaderSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoords;
        in vec4 Color;

        out vec4 FragColor;

        uniform vec3 lightDir;
        uniform vec3 lightColor;
        uniform vec3 viewPos;

        void main() {
            // Ambient
            float ambientStrength = 0.3;
            vec3 ambient = ambientStrength * lightColor;
            
            // Diffuse
            vec3 norm = normalize(Normal);
            vec3 lightDirection = normalize(lightDir);
            float diff = max(dot(norm, lightDirection), 0.0);
            vec3 diffuse = diff * lightColor;
            
            // Specular
            float specularStrength = 0.1;
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDirection, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor;
            
            vec3 result = (ambient + diffuse + specular) * Color.rgb;
            FragColor = vec4(result, Color.a);
        }
    )";

    m_Shader = std::make_unique<Shader>(vertexShaderSource, fragmentShaderSource);
    
    // Initialize color cache with alpha values
    m_ColorCache[VoxelType::Air] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    m_ColorCache[VoxelType::Grass] = glm::vec4(0.4f, 0.8f, 0.2f, 1.0f);
    m_ColorCache[VoxelType::Dirt] = glm::vec4(0.6f, 0.4f, 0.2f, 1.0f);
    m_ColorCache[VoxelType::Stone] = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    m_ColorCache[VoxelType::Sand] = glm::vec4(0.95f, 0.95f, 0.5f, 1.0f);
    m_ColorCache[VoxelType::Water] = glm::vec4(0.2f, 0.5f, 0.9f, 0.7f); // Semi-transparent
    m_ColorCache[VoxelType::Wood] = glm::vec4(0.5f, 0.3f, 0.1f, 1.0f);
    m_ColorCache[VoxelType::Leaves] = glm::vec4(0.2f, 0.6f, 0.1f, 0.9f); // Semi-transparent
}

void VoxelRenderer::addFace(const VoxelFace& face) {
    m_Faces.push_back(face);
}

void VoxelRenderer::clear() {
    m_Faces.clear();
    m_Vertices.clear();
    m_Indices.clear();
    m_Mesh.reset();
}

void VoxelRenderer::buildMesh() {
    if (m_Faces.empty()) {
        return;
    }
    
    // For debugging
    std::cout << "Building mesh with " << m_Faces.size() << " faces" << std::endl;
    
    // Clear previous mesh data
    m_Vertices.clear();
    m_Indices.clear();
    
    // Generate mesh data from faces
    unsigned int baseIndex = 0;
    for (const auto& face : m_Faces) {
        // Generate vertices for this face
        std::vector<Vertex> faceVertices = generateFaceVertices(face);
        m_Vertices.insert(m_Vertices.end(), faceVertices.begin(), faceVertices.end());
        
        // Generate indices for this face
        std::vector<unsigned int> faceIndices = generateFaceIndices(baseIndex);
        m_Indices.insert(m_Indices.end(), faceIndices.begin(), faceIndices.end());
        
        // Update base index for the next face
        baseIndex += 4; // Each face has 4 vertices
    }
    
    // Create mesh
    m_Mesh = std::make_unique<Mesh>(m_Vertices, m_Indices);
    
    // For debugging
    std::cout << "Created mesh with " << m_Vertices.size() << " vertices and " << m_Indices.size() << " indices" << std::endl;
}

void VoxelRenderer::render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    if (!m_Mesh || !m_Shader) {
        return;
    }
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Enable blending for transparent voxels
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Disable face culling completely for testing
    glDisable(GL_CULL_FACE);
    
    // Bind shader
    m_Shader->bind();
    
    // Set uniforms
    m_Shader->setMat4("model", glm::mat4(1.0f));
    m_Shader->setMat4("view", viewMatrix);
    m_Shader->setMat4("projection", projectionMatrix);
    
    // Set lighting uniforms
    m_Shader->setVec3("lightDir", glm::normalize(glm::vec3(0.5f, 1.0f, 0.3f)));
    m_Shader->setVec3("lightColor", glm::vec3(1.0f, 0.95f, 0.9f)); // Slightly warm sunlight color
    
    // Extract camera position from the view matrix
    glm::mat4 invView = glm::inverse(viewMatrix);
    glm::vec3 viewPos = glm::vec3(invView[3]);
    m_Shader->setVec3("viewPos", viewPos);
    
    // Render mesh
    m_Mesh->render();
    
    // Cleanup state
    glDisable(GL_BLEND);
    
    // Unbind shader
    m_Shader->unbind();
}

glm::vec4 VoxelRenderer::getVoxelColor(VoxelType type) const {
    auto it = m_ColorCache.find(type);
    if (it != m_ColorCache.end()) {
        return it->second;
    }
    
    // Default color (magenta) for unknown types
    return glm::vec4(1.0f, 0.0f, 1.0f, 1.0f);
}

std::vector<Vertex> VoxelRenderer::generateFaceVertices(const VoxelFace& face) const {
    float x = static_cast<float>(face.position.x) * m_VoxelSize;
    float y = static_cast<float>(face.position.y) * m_VoxelSize;
    float z = static_cast<float>(face.position.z) * m_VoxelSize;
    
    float s = m_VoxelSize;
    glm::vec4 color = getVoxelColor(face.type);
    
    std::vector<Vertex> vertices(4);
    
    // Set normals and positions based on face direction
    // Ensuring consistent counter-clockwise winding when viewed from outside
    switch (face.direction) {
        case VoxelFace::Direction::Front: // +Z
            vertices[0] = { glm::vec3(x,     y,     z + s), glm::vec3(0, 0, 1), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x + s, y,     z + s), glm::vec3(0, 0, 1), glm::vec2(1, 0), color };
            vertices[2] = { glm::vec3(x + s, y + s, z + s), glm::vec3(0, 0, 1), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x,     y + s, z + s), glm::vec3(0, 0, 1), glm::vec2(0, 1), color };
            break;
        case VoxelFace::Direction::Back: // -Z
            vertices[0] = { glm::vec3(x,     y,     z), glm::vec3(0, 0, -1), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x,     y + s, z), glm::vec3(0, 0, -1), glm::vec2(0, 1), color };
            vertices[2] = { glm::vec3(x + s, y + s, z), glm::vec3(0, 0, -1), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x + s, y,     z), glm::vec3(0, 0, -1), glm::vec2(1, 0), color };
            break;
        case VoxelFace::Direction::Top: // +Y
            vertices[0] = { glm::vec3(x,     y + s, z    ), glm::vec3(0, 1, 0), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x,     y + s, z + s), glm::vec3(0, 1, 0), glm::vec2(0, 1), color };
            vertices[2] = { glm::vec3(x + s, y + s, z + s), glm::vec3(0, 1, 0), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x + s, y + s, z    ), glm::vec3(0, 1, 0), glm::vec2(1, 0), color };
            break;
        case VoxelFace::Direction::Bottom: // -Y
            vertices[0] = { glm::vec3(x,     y, z    ), glm::vec3(0, -1, 0), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x + s, y, z    ), glm::vec3(0, -1, 0), glm::vec2(1, 0), color };
            vertices[2] = { glm::vec3(x + s, y, z + s), glm::vec3(0, -1, 0), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x,     y, z + s), glm::vec3(0, -1, 0), glm::vec2(0, 1), color };
            break;
        case VoxelFace::Direction::Right: // +X
            vertices[0] = { glm::vec3(x + s, y,     z    ), glm::vec3(1, 0, 0), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x + s, y + s, z    ), glm::vec3(1, 0, 0), glm::vec2(0, 1), color };
            vertices[2] = { glm::vec3(x + s, y + s, z + s), glm::vec3(1, 0, 0), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x + s, y,     z + s), glm::vec3(1, 0, 0), glm::vec2(1, 0), color };
            break;
        case VoxelFace::Direction::Left: // -X
            vertices[0] = { glm::vec3(x, y,     z + s), glm::vec3(-1, 0, 0), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x, y,     z    ), glm::vec3(-1, 0, 0), glm::vec2(1, 0), color };
            vertices[2] = { glm::vec3(x, y + s, z    ), glm::vec3(-1, 0, 0), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x, y + s, z + s), glm::vec3(-1, 0, 0), glm::vec2(0, 1), color };
            break;
    }
    
    return vertices;
}

std::vector<unsigned int> VoxelRenderer::generateFaceIndices(unsigned int baseIndex) const {
    return {
        baseIndex, baseIndex + 1, baseIndex + 2,
        baseIndex, baseIndex + 2, baseIndex + 3
    };
}

} // namespace VoxelEngine 