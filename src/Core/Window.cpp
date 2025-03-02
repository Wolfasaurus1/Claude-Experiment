#include "Window.hpp"
#include <iostream>

namespace VoxelEngine {

Window::Window(int width, int height, const std::string& title)
    : m_Title(title), m_Width(width), m_Height(height)
{
    // Set GLFW error callback
    glfwSetErrorCallback(glfwErrorCallback);

    // Initialize GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA

    // Create window
    m_Window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_Window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Make OpenGL context current
    glfwMakeContextCurrent(m_Window);

    // Set framebuffer size callback
    glfwSetWindowUserPointer(m_Window, this);
    glfwSetFramebufferSizeCallback(m_Window, glfwFramebufferSizeCallback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        throw std::runtime_error("Failed to initialize GLAD");
    }

    // Enable vsync
    glfwSwapInterval(1);
}

Window::~Window() {
    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_Window);
}

void Window::swapBuffers() {
    glfwSwapBuffers(m_Window);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::close() {
    glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
}

void Window::setFramebufferSizeCallback(const FramebufferSizeCallback& callback) {
    m_FramebufferSizeCallback = callback;
}

bool Window::isKeyPressed(int key) const {
    return glfwGetKey(m_Window, key) == GLFW_PRESS;
}

bool Window::isMouseButtonPressed(int button) const {
    return glfwGetMouseButton(m_Window, button) == GLFW_PRESS;
}

void Window::getCursorPosition(double& x, double& y) const {
    glfwGetCursorPos(m_Window, &x, &y);
}

void Window::setCursorPosition(double x, double y) {
    glfwSetCursorPos(m_Window, x, y);
}

void Window::enableCursor() {
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::disableCursor() {
    glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void Window::glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (instance) {
        instance->m_Width = width;
        instance->m_Height = height;
        
        if (instance->m_FramebufferSizeCallback) {
            instance->m_FramebufferSizeCallback(width, height);
        }
    }
}

} // namespace VoxelEngine 