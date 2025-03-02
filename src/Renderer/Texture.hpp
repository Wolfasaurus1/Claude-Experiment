#pragma once

#include <glad/glad.h>
#include <string>

namespace VoxelEngine {

class Texture {
public:
    enum class Type {
        Diffuse,
        Specular,
        Normal,
        Height
    };

    Texture(const std::string& path, Type type = Type::Diffuse);
    ~Texture();

    // Delete copy constructor and assignment operator
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    void bind(unsigned int slot = 0) const;
    void unbind() const;

    // Getters
    unsigned int getID() const { return m_ID; }
    int getWidth() const { return m_Width; }
    int getHeight() const { return m_Height; }
    int getChannels() const { return m_Channels; }
    Type getType() const { return m_Type; }

    // Static texture creation helpers
    static Texture* createFromColor(unsigned char r, unsigned char g, unsigned char b, Type type = Type::Diffuse);
    static Texture* createCheckerboard(int width, int height, Type type = Type::Diffuse);
    
private:
    unsigned int m_ID = 0;
    int m_Width = 0;
    int m_Height = 0;
    int m_Channels = 0;
    Type m_Type;
};

} // namespace VoxelEngine 