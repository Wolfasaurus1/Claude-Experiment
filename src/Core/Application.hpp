#pragma once

#include "Window.hpp"
#include "../Renderer/Camera.hpp"
#include <memory>
#include <string>

namespace VoxelEngine {

class Application {
public:
    Application(const std::string& name = "Voxel Engine", uint32_t width = 1280, uint32_t height = 720);
    virtual ~Application();

    // Delete copy constructor and assignment operator
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void run();
    void close();

protected:
    virtual void onInit() = 0;
    virtual void onUpdate(float deltaTime) = 0;
    virtual void onRender() = 0;
    virtual void onImGuiRender() {} // Optional ImGui rendering
    virtual void onShutdown() = 0;

    // Input event callbacks
    virtual void onKeyPressed(int key) {}
    virtual void onKeyReleased(int key) {}
    virtual void onMouseMoved(double xPos, double yPos) {}
    virtual void onMouseButtonPressed(int button) {}
    virtual void onMouseButtonReleased(int button) {}
    virtual void onMouseScrolled(double xOffset, double yOffset) {}
    virtual void onWindowResized(int width, int height) {}

    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Camera> m_Camera;

    bool m_Running = false;
    float m_LastFrameTime = 0.0f;
    
    // Mouse state
    bool m_FirstMouse = true;
    double m_LastMouseX = 0.0, m_LastMouseY = 0.0;
    
    // Input handling helpers
    void processInput();

private:
    void initWindow(const std::string& name, uint32_t width, uint32_t height);
    void initOpenGL();
    void initImGui();
    void shutdownImGui();
    
    // GLFW callbacks
    void framebufferSizeCallback(int width, int height);
};

} // namespace VoxelEngine 