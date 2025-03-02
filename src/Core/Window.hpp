#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <functional>

namespace VoxelEngine {

class Window {
public:
    using FramebufferSizeCallback = std::function<void(int, int)>;
    
    Window(int width, int height, const std::string& title);
    ~Window();

    // Delete copy constructor and assignment operator
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Core window functions
    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    void close();

    // Setters
    void setFramebufferSizeCallback(const FramebufferSizeCallback& callback);

    // Getters
    GLFWwindow* getGLFWWindow() const { return m_Window; }
    int getWidth() const { return m_Width; }
    int getHeight() const { return m_Height; }
    float getAspectRatio() const { return static_cast<float>(m_Width) / m_Height; }
    
    // Input handling
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    void getCursorPosition(double& x, double& y) const;
    void setCursorPosition(double x, double y);
    void enableCursor();
    void disableCursor();

private:
    static void glfwErrorCallback(int error, const char* description);
    static void glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_Window = nullptr;
    std::string m_Title;
    int m_Width = 0;
    int m_Height = 0;
    FramebufferSizeCallback m_FramebufferSizeCallback;
};

} // namespace VoxelEngine 