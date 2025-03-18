#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include "Shader.hpp"

namespace VoxelEngine {

class ShadowMap {
public:
    ShadowMap(unsigned int width = 1024, unsigned int height = 1024);
    ~ShadowMap();
    
    // Delete copy constructor and assignment operator
    ShadowMap(const ShadowMap&) = delete;
    ShadowMap& operator=(const ShadowMap&) = delete;
    
    // Begin shadow rendering pass
    void begin();
    
    // End shadow rendering pass
    void end();
    
    // Bind shadow map texture for sampling in the main render pass
    void bindTexture(unsigned int textureUnit = 0) const;
    
    // Get depth texture ID
    unsigned int getDepthTexture() const { return m_DepthTexture; }
    
    // Get light space matrix
    const glm::mat4& getLightSpaceMatrix() const { return m_LightSpaceMatrix; }
    
    // Update light space matrix
    void updateLightSpaceMatrix(const glm::vec3& lightDir, 
                               const glm::vec3& center,
                               float radius);
                               
    // Get shadow map shader
    Shader* getShadowShader() const { return m_ShadowShader.get(); }
    
private:
    // Create shadow map framebuffer and texture
    void createShadowMap();
    
    unsigned int m_Width;
    unsigned int m_Height;
    
    unsigned int m_DepthMapFBO = 0;
    unsigned int m_DepthTexture = 0;
    
    // Store original viewport dimensions
    GLint m_OldViewport[4] = {0, 0, 0, 0};
    
    glm::mat4 m_LightSpaceMatrix = glm::mat4(1.0f);
    
    // Shadow map shader for depth-only rendering
    std::unique_ptr<Shader> m_ShadowShader;
};

} // namespace VoxelEngine 