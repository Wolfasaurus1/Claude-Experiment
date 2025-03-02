#include "Texture.hpp"
#include <iostream>
#include <vector>

namespace VoxelEngine {

Texture::Texture(const std::string& path, Type type)
    : m_Type(type)
{
    // This is a placeholder implementation since we're not loading actual images
    // In a real application, you would use stb_image or another library to load textures
    
    // For now, let's just create a default white texture
    m_Width = 2;
    m_Height = 2;
    m_Channels = 4;
    
    // Create a simple 2x2 white texture
    unsigned char data[] = {
        255, 255, 255, 255,  255, 255, 255, 255,
        255, 255, 255, 255,  255, 255, 255, 255
    };
    
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Create texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    
    std::cout << "Created default texture with ID: " << m_ID << std::endl;
}

Texture::~Texture() {
    glDeleteTextures(1, &m_ID);
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture* Texture::createFromColor(unsigned char r, unsigned char g, unsigned char b, Type type) {
    // Create a new texture object
    Texture* texture = new Texture("", type);
    
    // Override the default texture with a solid color
    texture->m_Width = 1;
    texture->m_Height = 1;
    texture->m_Channels = 4;
    
    // Create a 1x1 texture with the specified color
    unsigned char data[] = { r, g, b, 255 };
    
    glBindTexture(GL_TEXTURE_2D, texture->m_ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
}

Texture* Texture::createCheckerboard(int width, int height, Type type) {
    // Create a new texture object
    Texture* texture = new Texture("", type);
    
    // Override the default texture with a checkerboard pattern
    texture->m_Width = width;
    texture->m_Height = height;
    texture->m_Channels = 4;
    
    // Create a checkerboard pattern
    std::vector<unsigned char> data(width * height * 4);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate pixel index
            int index = (y * width + x) * 4;
            
            // Set color based on checkerboard pattern
            bool isWhite = ((x + y) % 2 == 0);
            
            if (isWhite) {
                data[index + 0] = 255; // R
                data[index + 1] = 255; // G
                data[index + 2] = 255; // B
                data[index + 3] = 255; // A
            } else {
                data[index + 0] = 0;   // R
                data[index + 1] = 0;   // G
                data[index + 2] = 0;   // B
                data[index + 3] = 255; // A
            }
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, texture->m_ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return texture;
}

} // namespace VoxelEngine 