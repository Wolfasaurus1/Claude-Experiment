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
    // Initialize shadow map with higher resolution
    m_ShadowMap = std::make_unique<ShadowMap>(4096, 4096);
    
    // Initialize shader with shadow mapping support
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
        out vec4 FragPosLightSpace;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat4 lightSpaceMatrix;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoords = aTexCoords;
            Color = aColor;
            
            // Calculate position in light space for shadow mapping
            FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
            
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const std::string fragmentShaderSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoords;
        in vec4 Color;
        in vec4 FragPosLightSpace;

        out vec4 FragColor;

        uniform vec3 lightDir;
        uniform vec3 lightColor;
        uniform vec3 viewPos;
        uniform sampler2D shadowMap;
        uniform bool shadowsEnabled;

        // Calculate shadow factor
        float calculateShadow(vec4 fragPosLightSpace) {
            // Perform perspective divide
            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            
            // Transform to [0,1] range
            projCoords = projCoords * 0.5 + 0.5;
            
            // Check if position is outside the far plane or out of shadow map bounds
            if(projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) {
                return 0.0;
            }
            
            // Get closest depth value from light's perspective
            float closestDepth = texture(shadowMap, projCoords.xy).r; 
            
            // Get current depth value
            float currentDepth = projCoords.z;
            
            // Calculate bias based on depth map resolution and slope
            vec3 normal = normalize(Normal);
            vec3 lightDirection = normalize(lightDir);
            float cosTheta = max(dot(normal, lightDirection), 0.0);
            float bias = max(0.0003 * (1.0 - cosTheta), 0.00005);
            
            // For very steep angles (sun near horizon), increase bias slightly
            if (cosTheta < 0.1) {
                bias *= 3.0;
            }
            
            // Check whether current fragment is in shadow
            float shadow = 0.0;
            
            // PCF (Percentage-Closer Filtering)
            float shadowValue = 0.0;
            vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
            for(int x = -2; x <= 2; ++x) {
                for(int y = -2; y <= 2; ++y) {
                    float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
                    shadowValue += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
                }    
            }
            shadow = shadowValue / 25.0; // Using a 5x5 PCF kernel
            
            return shadow;
        }

        void main() {
            // Calculate light intensity based on direction (day/night cycle)
            // Light pointing straight down (noon) will be brightest
            float lightIntensityFactor = max(0.2, -lightDir.y);
            vec3 adjustedLightColor = lightColor * lightIntensityFactor;
            
            // Ambient: Adjust based on time of day
            // Higher ambient at noon, lower at night
            float timeOfDay = max(0.2, -lightDir.y); // 0.0 = night, 1.0 = noon
            float ambientStrength = mix(0.15, 0.4, timeOfDay);
            vec3 ambient = ambientStrength * adjustedLightColor;
            
            // Diffuse: Calculate using a normalized normal and light direction.
            vec3 norm = normalize(Normal);
            vec3 lightDirection = normalize(lightDir);
            float diff = max(dot(norm, lightDirection), 0.0);
            vec3 diffuse = diff * adjustedLightColor;
            
            // Specular: Use a lower exponent and reduced intensity for softer highlights.
            float specularStrength = 0.05 * lightIntensityFactor;
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDirection, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 10);
            vec3 specular = specularStrength * spec * adjustedLightColor;
            
            // Calculate shadow
            float shadow = shadowsEnabled ? calculateShadow(FragPosLightSpace) : 0.0;
            
            // Make shadows more pronounced during daytime and lighter during night
            float shadowIntensity = mix(0.5, 2.0, timeOfDay);
            shadow = min(shadow * shadowIntensity, 0.85);
            
            // Apply a slight ambient occlusion effect in shadowed areas
            float ambientOcclusion = 1.0 - shadow * 0.3;
            
            // Combine lighting with vertex color and shadow
            vec3 result = (ambient * ambientOcclusion + (1.0 - shadow) * (diffuse + specular)) * Color.rgb;
            
            // Apply gamma correction for a more natural appearance.
            float gamma = 2.2;
            result = pow(result, vec3(1.0 / gamma));
            
            FragColor = vec4(result, Color.a);
        }
    )";

    m_Shader = std::make_unique<Shader>(vertexShaderSource, fragmentShaderSource);
    
    // Initialize color cache with alpha values
    m_ColorCache[VoxelType::Air] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    m_ColorCache[VoxelType::Grass] = glm::vec4(0.063,0.192,0.243, 1.0f);
    m_ColorCache[VoxelType::Dirt] = glm::vec4(0.6f, 0.4f, 0.2f, 1.0f);
    m_ColorCache[VoxelType::Stone] = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    m_ColorCache[VoxelType::Sand] = glm::vec4(0.95f, 0.95f, 0.5f, 1.0f);
    m_ColorCache[VoxelType::Water] = glm::vec4(0.184,0.478,0.471, 0.7f); // Semi-transparent
    m_ColorCache[VoxelType::Wood] = glm::vec4(0.275,0.573,0.502, 1.0f);
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

void VoxelRenderer::buildGreedyMesh(int chunkSizeX, int chunkSizeY, int chunkSizeZ, 
                                   const std::function<VoxelType(int, int, int)>& getVoxelFunc,
                                   const std::function<bool(int, int, int, VoxelFace::Direction)>& shouldRenderFaceFunc,
                                   const glm::ivec3& chunkPos) {
    // Clear any existing mesh data
    m_Faces.clear();
    m_MergedFaces.clear();
    m_Vertices.clear();
    m_Indices.clear();
    
    // For performance tracking
    int totalFacesBeforeMerging = 0;
    int totalFacesAfterMerging = 0;
    
    // We'll process each direction separately
    for (int faceDir = 0; faceDir < 6; faceDir++) {
        VoxelFace::Direction direction = static_cast<VoxelFace::Direction>(faceDir);
        
        // Determine which dimensions to iterate based on the current face direction
        int dimU = 0, dimV = 0, dimW = 0; // Initialize to prevent warnings
        
        switch (direction) {
            case VoxelFace::Direction::Front:  // +Z
            case VoxelFace::Direction::Back:   // -Z
                dimU = 0; dimV = 1; dimW = 2; // X, Y, Z
                break;
            case VoxelFace::Direction::Top:    // +Y
            case VoxelFace::Direction::Bottom: // -Y
                dimU = 0; dimV = 2; dimW = 1; // X, Z, Y
                break;
            case VoxelFace::Direction::Right:  // +X
            case VoxelFace::Direction::Left:   // -X
                dimU = 2; dimV = 1; dimW = 0; // Z, Y, X
                break;
        }
        
        // Get the dimensions of the chunk
        int uSize = 0, vSize = 0, wSize = 0; // Initialize to prevent warnings
        
        switch (dimU) {
            case 0: uSize = chunkSizeX; break;
            case 1: uSize = chunkSizeY; break;
            case 2: uSize = chunkSizeZ; break;
        }
        switch (dimV) {
            case 0: vSize = chunkSizeX; break;
            case 1: vSize = chunkSizeY; break;
            case 2: vSize = chunkSizeZ; break;
        }
        switch (dimW) {
            case 0: wSize = chunkSizeX; break;
            case 1: wSize = chunkSizeY; break;
            case 2: wSize = chunkSizeZ; break;
        }
        
        // For each layer (perpendicular to the face direction)
        for (int w = 0; w < wSize; w++) {
            // Create a mask for the current slice
            std::vector<std::vector<bool>> mask(vSize, std::vector<bool>(uSize, false));
            std::vector<std::vector<VoxelType>> types(vSize, std::vector<VoxelType>(uSize, VoxelType::Air));
            
            // Fill the mask for the current slice
            for (int v = 0; v < vSize; v++) {
                for (int u = 0; u < uSize; u++) {
                    // Convert from uvw coordinates to xyz coordinates
                    glm::ivec3 pos(0, 0, 0);
                    
                    // Properly map UVW to XYZ without overwriting
                    if (dimU == 0) pos.x = u;
                    else if (dimU == 1) pos.y = u;
                    else if (dimU == 2) pos.z = u;
                    
                    if (dimV == 0) pos.x = v;
                    else if (dimV == 1) pos.y = v;
                    else if (dimV == 2) pos.z = v;
                    
                    if (dimW == 0) pos.x = w;
                    else if (dimW == 1) pos.y = w;
                    else if (dimW == 2) pos.z = w;
                    
                    // Check if a face should be rendered at this position
                    if (shouldRenderFaceFunc(pos.x, pos.y, pos.z, direction)) {
                        mask[v][u] = true;
                        types[v][u] = getVoxelFunc(pos.x, pos.y, pos.z);
                        totalFacesBeforeMerging++;
                    }
                }
            }
            
            // Now, greedily find rectangles in the mask
            for (int v = 0; v < vSize; v++) {
                for (int u = 0; u < uSize; u++) {
                    if (mask[v][u]) {
                        // Found an unprocessed face, try to expand it
                        VoxelType currentType = types[v][u];
                        
                        // Find the width (extend in u direction)
                        int width = 1;
                        while (u + width < uSize && 
                               mask[v][u + width] && 
                               types[v][u + width] == currentType) {
                            width++;
                        }
                        
                        // Find the height (extend in v direction)
                        int height = 1;
                        bool canExtend = true;
                        
                        while (canExtend && v + height < vSize) {
                            // Check if we can extend the entire width
                            for (int du = 0; du < width; du++) {
                                if (!mask[v + height][u + du] || 
                                    types[v + height][u + du] != currentType) {
                                    canExtend = false;
                                    break;
                                }
                            }
                            
                            if (canExtend) {
                                height++;
                            }
                        }
                        
                        // Create a merged face
                        MergedFace mergedFace;
                        mergedFace.direction = direction;
                        mergedFace.type = currentType;
                        
                        // Convert from uvw to xyz for the starting position
                        glm::ivec3 start(0, 0, 0);
                        
                        // Properly map UVW to XYZ for the start position
                        if (dimU == 0) start.x = u;
                        else if (dimU == 1) start.y = u;
                        else if (dimU == 2) start.z = u;
                        
                        if (dimV == 0) start.x = v;
                        else if (dimV == 1) start.y = v;
                        else if (dimV == 2) start.z = v;
                        
                        if (dimW == 0) start.x = w;
                        else if (dimW == 1) start.y = w;
                        else if (dimW == 2) start.z = w;
                        
                        // Also store the dimensional information for correct vertex generation
                        mergedFace.size = glm::ivec2(width, height);
                        
                        // Calculate the actual world position
                        mergedFace.start = start + chunkPos * glm::ivec3(chunkSizeX, 0, chunkSizeZ);
                        
                        // Add the merged face
                        m_MergedFaces.push_back(mergedFace);
                        totalFacesAfterMerging++;
                        
                        // Mark the merged area as processed
                        for (int dv = 0; dv < height; dv++) {
                            for (int du = 0; du < width; du++) {
                                mask[v + dv][u + du] = false;
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Generate vertices and indices for all merged faces
    unsigned int baseIndex = 0;
    for (const auto& face : m_MergedFaces) {
        std::vector<Vertex> faceVertices = generateMergedFaceVertices(face);
        m_Vertices.insert(m_Vertices.end(), faceVertices.begin(), faceVertices.end());
        
        std::vector<unsigned int> faceIndices = generateFaceIndices(baseIndex);
        m_Indices.insert(m_Indices.end(), faceIndices.begin(), faceIndices.end());
        
        baseIndex += 4; // Each face still has 4 vertices
    }
    
    // Create mesh
    m_Mesh = std::make_unique<Mesh>(m_Vertices, m_Indices);
    
    // Print statistics
    std::cout << "Greedy meshing: " << totalFacesBeforeMerging << " faces reduced to " 
              << totalFacesAfterMerging << " (" 
              << (totalFacesBeforeMerging > 0 ? (100 - totalFacesAfterMerging * 100 / totalFacesBeforeMerging) : 0) 
              << "% reduction)" << std::endl;
}

void VoxelRenderer::renderMeshOnly(Shader* shader) {
    if (!m_Mesh || !shader) {
        return;
    }
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Bind shader
    shader->bind();
    
    // Set model matrix to identity (handled by the caller if needed)
    shader->setMat4("model", glm::mat4(1.0f));
    
    // Render mesh geometry
    m_Mesh->render();
    
    // Unbind shader
    shader->unbind();
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
    m_Shader->setVec3("lightDir", m_LightDir);
    m_Shader->setVec3("lightColor", m_LightColor);
    
    // Extract camera position from the view matrix
    glm::mat4 invView = glm::inverse(viewMatrix);
    glm::vec3 viewPos = glm::vec3(invView[3]);
    m_Shader->setVec3("viewPos", viewPos);
    
    // Set shadow mapping uniforms
    m_Shader->setInt("shadowMap", 0);
    m_Shader->setBool("shadowsEnabled", m_ShadowsEnabled);
    
    // Set light space matrix - use the matrix that was set by the game
    m_Shader->setMat4("lightSpaceMatrix", m_LightSpaceMatrix);
    
    // Render mesh
    m_Mesh->render();
    
    // Cleanup state
    glDisable(GL_BLEND);
    
    // Unbind shader
    m_Shader->unbind();
}

void VoxelRenderer::renderShadowPass() {
    if (!m_Mesh || !m_ShadowMap) {
        return;
    }
    
    // Begin shadow pass
    m_ShadowMap->begin();
    
    // Use shadow shader and set model matrix
    m_ShadowMap->getShadowShader()->setMat4("model", glm::mat4(1.0f));
    
    // Render mesh
    m_Mesh->render();
    
    // End shadow pass
    m_ShadowMap->end();
}

void VoxelRenderer::updateLight(const glm::vec3& lightDir, const glm::vec3& center, float radius) {
    m_LightDir = glm::normalize(lightDir);
    
    if (m_ShadowMap) {
        m_ShadowMap->updateLightSpaceMatrix(m_LightDir, center, radius);
    }
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

std::vector<Vertex> VoxelRenderer::generateMergedFaceVertices(const MergedFace& face) const {
    float x = static_cast<float>(face.start.x) * m_VoxelSize;
    float y = static_cast<float>(face.start.y) * m_VoxelSize;
    float z = static_cast<float>(face.start.z) * m_VoxelSize;
    
    float width = static_cast<float>(face.size.x) * m_VoxelSize;
    float height = static_cast<float>(face.size.y) * m_VoxelSize;
    
    glm::vec4 color = getVoxelColor(face.type);
    
    std::vector<Vertex> vertices(4);
    
    // Set normals and positions based on face direction
    // Use the size of the merged face for the dimensions
    switch (face.direction) {
        case VoxelFace::Direction::Front: // +Z
            vertices[0] = { glm::vec3(x,         y,          z + m_VoxelSize), glm::vec3(0, 0, 1), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x + width, y,          z + m_VoxelSize), glm::vec3(0, 0, 1), glm::vec2(1, 0), color };
            vertices[2] = { glm::vec3(x + width, y + height, z + m_VoxelSize), glm::vec3(0, 0, 1), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x,         y + height, z + m_VoxelSize), glm::vec3(0, 0, 1), glm::vec2(0, 1), color };
            break;
        case VoxelFace::Direction::Back: // -Z
            vertices[0] = { glm::vec3(x,         y,          z), glm::vec3(0, 0, -1), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x,         y + height, z), glm::vec3(0, 0, -1), glm::vec2(0, 1), color };
            vertices[2] = { glm::vec3(x + width, y + height, z), glm::vec3(0, 0, -1), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x + width, y,          z), glm::vec3(0, 0, -1), glm::vec2(1, 0), color };
            break;
        case VoxelFace::Direction::Top: // +Y
            vertices[0] = { glm::vec3(x,          y + m_VoxelSize, z         ), glm::vec3(0, 1, 0), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x,          y + m_VoxelSize, z + height), glm::vec3(0, 1, 0), glm::vec2(0, 1), color };
            vertices[2] = { glm::vec3(x + width,  y + m_VoxelSize, z + height), glm::vec3(0, 1, 0), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x + width,  y + m_VoxelSize, z         ), glm::vec3(0, 1, 0), glm::vec2(1, 0), color };
            break;
        case VoxelFace::Direction::Bottom: // -Y
            vertices[0] = { glm::vec3(x,         y, z         ), glm::vec3(0, -1, 0), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x + width, y, z         ), glm::vec3(0, -1, 0), glm::vec2(1, 0), color };
            vertices[2] = { glm::vec3(x + width, y, z + height), glm::vec3(0, -1, 0), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x,         y, z + height), glm::vec3(0, -1, 0), glm::vec2(0, 1), color };
            break;
        case VoxelFace::Direction::Right: // +X
            vertices[0] = { glm::vec3(x + m_VoxelSize, y,          z         ), glm::vec3(1, 0, 0), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x + m_VoxelSize, y + height, z         ), glm::vec3(1, 0, 0), glm::vec2(0, 1), color };
            vertices[2] = { glm::vec3(x + m_VoxelSize, y + height, z + width ), glm::vec3(1, 0, 0), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x + m_VoxelSize, y,          z + width ), glm::vec3(1, 0, 0), glm::vec2(1, 0), color };
            break;
        case VoxelFace::Direction::Left: // -X
            vertices[0] = { glm::vec3(x, y,          z + width ), glm::vec3(-1, 0, 0), glm::vec2(0, 0), color };
            vertices[1] = { glm::vec3(x, y,          z         ), glm::vec3(-1, 0, 0), glm::vec2(1, 0), color };
            vertices[2] = { glm::vec3(x, y + height, z         ), glm::vec3(-1, 0, 0), glm::vec2(1, 1), color };
            vertices[3] = { glm::vec3(x, y + height, z + width ), glm::vec3(-1, 0, 0), glm::vec2(0, 1), color };
            break;
    }
    
    return vertices;
}

} // namespace VoxelEngine 