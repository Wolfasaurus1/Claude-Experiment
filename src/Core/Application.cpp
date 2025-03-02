#include "Application.hpp"
#include <GLFW/glfw3.h>
#include <iostream>

namespace VoxelEngine {

Application::Application(const std::string& name, uint32_t width, uint32_t height) {
    initWindow(name, width, height);
    initOpenGL();
    
    // Create camera
    m_Camera = std::make_unique<Camera>(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);
    m_Camera->setPosition(glm::vec3(0.0f, 0.0f, 3.0f));
}

Application::~Application() {
    // Note: We don't call onShutdown() here anymore
    // The derived class should handle cleanup in its own destructor
    // This avoids calling virtual functions from destructors
}

void Application::run() {
    m_Running = true;
    onInit();
    
    while (m_Running && !m_Window->shouldClose()) {
        // Calculate delta time
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - m_LastFrameTime;
        m_LastFrameTime = currentTime;
        
        // Handle input
        processInput();
        
        // Update
        onUpdate(deltaTime);
        
        // Render
        onRender();
        
        // ImGui render (if applicable)
        onImGuiRender();
        
        // Swap buffers and poll events
        m_Window->swapBuffers();
        m_Window->pollEvents();
    }
    
    // Call onShutdown here instead of in the destructor
    onShutdown();
}

void Application::close() {
    m_Running = false;
}

void Application::processInput() {
    // Process keyboard input
    if (m_Window->isKeyPressed(GLFW_KEY_ESCAPE)) {
        close();
    }
    
    // Camera movement
    const float cameraSpeed = 5.0f;
    
    if (m_Window->isKeyPressed(GLFW_KEY_W)) {
        m_Camera->moveForward(cameraSpeed * 0.01f);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_S)) {
        m_Camera->moveForward(-cameraSpeed * 0.01f);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_A)) {
        m_Camera->moveRight(-cameraSpeed * 0.01f);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_D)) {
        m_Camera->moveRight(cameraSpeed * 0.01f);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_SPACE)) {
        m_Camera->moveUp(cameraSpeed * 0.01f);
    }
    if (m_Window->isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        m_Camera->moveUp(-cameraSpeed * 0.01f);
    }
    
    // Process mouse input for camera look
    if (m_Window->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        double xpos, ypos;
        m_Window->getCursorPosition(xpos, ypos);
        
        if (m_FirstMouse) {
            m_LastMouseX = xpos;
            m_LastMouseY = ypos;
            m_FirstMouse = false;
        }
        
        double xoffset = xpos - m_LastMouseX;
        double yoffset = m_LastMouseY - ypos; // Reversed since y-coordinates go from bottom to top
        
        m_LastMouseX = xpos;
        m_LastMouseY = ypos;
        
        m_Camera->processMouseMovement(static_cast<float>(xoffset), static_cast<float>(yoffset));
    } else {
        m_FirstMouse = true;
    }
}

void Application::initWindow(const std::string& name, uint32_t width, uint32_t height) {
    m_Window = std::make_unique<Window>(width, height, name);
    
    // Set window callbacks
    m_Window->setFramebufferSizeCallback([this](int width, int height) {
        framebufferSizeCallback(width, height);
    });
}

void Application::initOpenGL() {
    // Set OpenGL state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Enable MSAA
    glEnable(GL_MULTISAMPLE);
    
    // Set clear color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void Application::initImGui() {
    // Placeholder for ImGui initialization
    // Not implementing yet to keep things simple
}

void Application::shutdownImGui() {
    // Placeholder for ImGui shutdown
}

void Application::framebufferSizeCallback(int width, int height) {
    glViewport(0, 0, width, height);
    
    if (m_Camera) {
        m_Camera->setProjectionMatrix(45.0f, static_cast<float>(width) / height, 0.1f, 1000.0f);
    }
    
    onWindowResized(width, height);
}

} // namespace VoxelEngine 