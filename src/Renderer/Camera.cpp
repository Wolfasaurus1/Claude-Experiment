#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

namespace VoxelEngine {

Camera::Camera(float fov, float aspectRatio, float nearClip, float farClip)
    : m_Fov(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip)
{
    updateCameraVectors();
    updateViewMatrix();
    setProjectionMatrix(fov, aspectRatio, nearClip, farClip);
}

void Camera::update(float deltaTime) {
    // This is a placeholder. In a real application, you might update camera position based on physics or other factors.
}

void Camera::setPosition(const glm::vec3& position) {
    m_Position = position;
    updateViewMatrix();
}

void Camera::setRotation(float yaw, float pitch) {
    m_Yaw = yaw;
    m_Pitch = pitch;
    updateCameraVectors();
    updateViewMatrix();
}

void Camera::setProjectionType(ProjectionType type) {
    m_ProjectionType = type;
    setProjectionMatrix(m_Fov, m_AspectRatio, m_NearClip, m_FarClip);
}

void Camera::setProjectionMatrix(float fov, float aspectRatio, float nearClip, float farClip) {
    m_Fov = fov;
    m_AspectRatio = aspectRatio;
    m_NearClip = nearClip;
    m_FarClip = farClip;
    
    if (m_ProjectionType == ProjectionType::Perspective) {
        m_ProjectionMatrix = glm::perspective(glm::radians(m_Fov), m_AspectRatio, m_NearClip, m_FarClip);
    } else {
        // Orthographic projection (useful for 2D views or UI)
        float orthoSize = 10.0f;
        m_ProjectionMatrix = glm::ortho(-orthoSize * m_AspectRatio, orthoSize * m_AspectRatio, -orthoSize, orthoSize, m_NearClip, m_FarClip);
    }
}

void Camera::moveForward(float distance) {
    m_Position += m_Front * distance;
    updateViewMatrix();
}

void Camera::moveRight(float distance) {
    m_Position += m_Right * distance;
    updateViewMatrix();
}

void Camera::moveUp(float distance) {
    m_Position += m_Up * distance;
    updateViewMatrix();
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= m_MouseSensitivity;
    yOffset *= m_MouseSensitivity;

    m_Yaw += xOffset;
    m_Pitch += yOffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);
    }

    // Update camera vectors using the updated Euler angles
    updateCameraVectors();
    updateViewMatrix();
}

void Camera::processMouseScroll(float yOffset) {
    m_Zoom -= yOffset;
    m_Zoom = std::clamp(m_Zoom, 1.0f, 45.0f);
    
    // Update projection matrix with new zoom (FOV)
    setProjectionMatrix(m_Zoom, m_AspectRatio, m_NearClip, m_FarClip);
}

void Camera::updateViewMatrix() {
    m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

void Camera::updateCameraVectors() {
    // Calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);
    
    // Re-calculate the Right and Up vector
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

} // namespace VoxelEngine 