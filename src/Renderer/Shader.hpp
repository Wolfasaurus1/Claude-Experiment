#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace VoxelEngine {

class Shader {
public:
    Shader(const std::string& vertexSource, const std::string& fragmentSource);
    ~Shader();

    // Delete copy constructor and assignment operator
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void bind() const;
    void unbind() const;

    // Set uniforms
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec2(const std::string& name, const glm::vec2& value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setVec4(const std::string& name, const glm::vec4& value);
    void setMat3(const std::string& name, const glm::mat3& value);
    void setMat4(const std::string& name, const glm::mat4& value);

    // Utility
    static Shader* fromFile(const std::string& vertexPath, const std::string& fragmentPath);

private:
    // Compile and link shaders
    void compile(const std::string& vertexSource, const std::string& fragmentSource);
    
    // Get uniform location with caching
    GLint getUniformLocation(const std::string& name);

    // Static utility functions
    static std::string readFile(const std::string& filepath);
    static void checkCompileErrors(GLuint shader, const std::string& type);
    
    GLuint m_ShaderID = 0;
    std::unordered_map<std::string, GLint> m_UniformLocationCache;
};

} // namespace VoxelEngine 