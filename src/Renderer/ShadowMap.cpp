#include "ShadowMap.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace VoxelEngine {

ShadowMap::ShadowMap(unsigned int width, unsigned int height)
    : m_Width(width), m_Height(height) {
    // Create shadow map and shader
    createShadowMap();
    
    // Create shadow shader
    const std::string shadowVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 lightSpaceMatrix;
        uniform mat4 model;
        
        void main() {
            gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
        }
    )";
    
    const std::string shadowFragmentShader = R"(
        #version 330 core
        void main() {
            // No color output needed for shadow depth
            // Depth is automatically written to the depth buffer
        }
    )";
    
    // Create shadow shader
    m_ShadowShader = std::make_unique<Shader>(shadowVertexShader, shadowFragmentShader);
}

ShadowMap::~ShadowMap() {
    // Cleanup OpenGL resources
    if (m_DepthMapFBO != 0) {
        glDeleteFramebuffers(1, &m_DepthMapFBO);
    }
    if (m_DepthTexture != 0) {
        glDeleteTextures(1, &m_DepthTexture);
    }
}

void ShadowMap::createShadowMap() {
    // Create depth texture
    glGenTextures(1, &m_DepthTexture);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    // Set border color to white (1.0)
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Create framebuffer
    glGenFramebuffers(1, &m_DepthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DepthTexture, 0);
    
    // We don't need color buffer for shadow map
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Failed to create shadow map framebuffer" << std::endl;
    }
    
    // Unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::begin() {
    // Store current viewport before changing it
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    m_OldViewport[0] = viewport[0];
    m_OldViewport[1] = viewport[1];
    m_OldViewport[2] = viewport[2];
    m_OldViewport[3] = viewport[3];
    
    // Bind framebuffer and set viewport
    glBindFramebuffer(GL_FRAMEBUFFER, m_DepthMapFBO);
    glViewport(0, 0, m_Width, m_Height);
    
    // Clear depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Enable depth test and polygon offset
    glEnable(GL_DEPTH_TEST);
    
    // Use polygon offset to reduce shadow acne
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(4.0f, 4.0f);
    
    // Bind shadow shader
    m_ShadowShader->bind();
    m_ShadowShader->setMat4("lightSpaceMatrix", m_LightSpaceMatrix);
}

void ShadowMap::end() {
    // Unbind framebuffer and restore viewport
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Restore previous viewport
    glViewport(m_OldViewport[0], m_OldViewport[1], m_OldViewport[2], m_OldViewport[3]);
    
    // Disable polygon offset
    glDisable(GL_POLYGON_OFFSET_FILL);
    
    // Unbind shader
    m_ShadowShader->unbind();
}

void ShadowMap::bindTexture(unsigned int textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(GL_TEXTURE_2D, m_DepthTexture);
}

void ShadowMap::updateLightSpaceMatrix(const glm::vec3& lightDir, 
                                     const glm::vec3& center, 
                                     float radius) {
    // Direction should be normalized
    glm::vec3 lightDirection = glm::normalize(lightDir);
    
    // For sun-like lighting, position the light source 
    // by moving OPPOSITE to the light direction
    // (since lightDir points FROM light TO scene)
    glm::vec3 lightPosition = center + lightDirection * (radius * 3.0f);
    
    // Create light view matrix (looking from light's perspective toward center)
    glm::mat4 lightView = glm::lookAt(
        lightPosition,
        center,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    // Create orthographic projection for directional light
    // For sun-like lighting, we need a wider frustum to capture longer shadows
    float orthoSize = radius * 2.2f;
    glm::mat4 lightProjection = glm::ortho(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        0.1f, radius * 8.0f
    );
    
    // Combine to create the light space matrix
    m_LightSpaceMatrix = lightProjection * lightView;
}

} // namespace VoxelEngine 