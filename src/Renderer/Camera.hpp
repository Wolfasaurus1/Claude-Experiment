#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace VoxelEngine {

class Camera {
public:
    enum class ProjectionType {
        Perspective,
        Orthographic
    };

    Camera(float fov, float aspectRatio, float nearClip, float farClip);

    // Update camera orientation and position
    void update(float deltaTime);
    
    // Getters
    const glm::mat4& getViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return m_ProjectionMatrix; }
    const glm::vec3& getPosition() const { return m_Position; }
    const glm::vec3& getFront() const { return m_Front; }
    float getYaw() const { return m_Yaw; }
    float getPitch() const { return m_Pitch; }
    float getZoom() const { return m_Zoom; }
    
    // Setters
    void setPosition(const glm::vec3& position);
    void setRotation(float yaw, float pitch);
    void setProjectionType(ProjectionType type);
    void setProjectionMatrix(float fov, float aspectRatio, float nearClip, float farClip);
    
    // Camera movement
    void moveForward(float distance);
    void moveRight(float distance);
    void moveUp(float distance);
    
    // Mouse look
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void processMouseScroll(float yOffset);

private:
    // Updates the view matrix based on camera position and orientation
    void updateViewMatrix();
    // Updates the camera vectors (front, right, up) based on yaw and pitch
    void updateCameraVectors();
    
    // Camera attributes
    glm::vec3 m_Position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Euler angles
    float m_Yaw = -90.0f;
    float m_Pitch = 0.0f;
    
    // Camera options
    float m_MovementSpeed = 5.0f;
    float m_MouseSensitivity = 0.1f;
    float m_Zoom = 45.0f;
    
    // Near and far clip planes
    float m_NearClip = 0.1f;
    float m_FarClip = 1000.0f;
    
    // Aspect ratio
    float m_AspectRatio = 1.778f;
    
    // FOV (Field of View)
    float m_Fov = 45.0f;
    
    // Projection type
    ProjectionType m_ProjectionType = ProjectionType::Perspective;
    
    // Matrices
    glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
    glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
};

} // namespace VoxelEngine 