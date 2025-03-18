#include "Shader.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

namespace VoxelEngine {

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource) {
    compile(vertexSource, fragmentSource);
}

Shader::~Shader() {
    glDeleteProgram(m_ShaderID);
}

void Shader::compile(const std::string& vertexSource, const std::string& fragmentSource) {
    // Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* source = vertexSource.c_str();
    glShaderSource(vertexShader, 1, &source, nullptr);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    // Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    source = fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &source, nullptr);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    // Shader Program
    m_ShaderID = glCreateProgram();
    glAttachShader(m_ShaderID, vertexShader);
    glAttachShader(m_ShaderID, fragmentShader);
    glLinkProgram(m_ShaderID);
    checkCompileErrors(m_ShaderID, "PROGRAM");

    // Delete the shaders as they're linked into our program and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::bind() const {
    glUseProgram(m_ShaderID);
}

void Shader::unbind() const {
    glUseProgram(0);
}

void Shader::setBool(const std::string& name, bool value) {
    glUniform1i(getUniformLocation(name), static_cast<int>(value));
}

void Shader::setInt(const std::string& name, int value) {
    glUniform1i(getUniformLocation(name), value);
}

void Shader::setFloat(const std::string& name, float value) {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setVec2(const std::string& name, const glm::vec2& value) {
    glUniform2fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setVec4(const std::string& name, const glm::vec4& value) {
    glUniform4fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setMat3(const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

GLint Shader::getUniformLocation(const std::string& name) {
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
        return m_UniformLocationCache[name];
    }

    GLint location = glGetUniformLocation(m_ShaderID, name.c_str());
    if (location == -1) {
        std::cout << "Warning: uniform '" << name << "' doesn't exist!" << std::endl;
    }

    m_UniformLocationCache[name] = location;
    return location;
}

Shader* Shader::fromFile(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = readFile(vertexPath);
    std::string fragmentSource = readFile(fragmentPath);
    return new Shader(vertexSource, fragmentSource);
}

std::string Shader::readFile(const std::string& filepath) {
    std::string content;
    std::ifstream fileStream(filepath, std::ios::in);

    if (!fileStream.is_open()) {
        std::cerr << "Could not read file " << filepath << ". File does not exist." << std::endl;
        return "";
    }

    std::stringstream sstr;
    sstr << fileStream.rdbuf();
    content = sstr.str();
    fileStream.close();
    
    return content;
}

void Shader::checkCompileErrors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

} // namespace VoxelEngine 