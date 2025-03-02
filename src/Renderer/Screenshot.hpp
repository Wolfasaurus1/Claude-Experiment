#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace VoxelEngine {

class Screenshot {
public:
    // Capture current OpenGL framebuffer and save as PNG
    static bool capture(const std::string& filename) {
        // Get the current viewport dimensions
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        
        int width = viewport[2];
        int height = viewport[3];
        
        // Allocate memory for the pixels
        std::vector<unsigned char> pixels(width * height * 4);
        
        // Read the framebuffer pixels
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
        
        // Flip the image vertically since OpenGL has the origin at the bottom-left
        for (int y = 0; y < height / 2; y++) {
            for (int x = 0; x < width * 4; x++) {
                std::swap(pixels[y * width * 4 + x], pixels[(height - 1 - y) * width * 4 + x]);
            }
        }
        
        // Write raw data to file (BMP format for simplicity without dependencies)
        return writeBMP(filename, width, height, pixels.data());
    }

private:
    // Write BMP file format (simple implementation that doesn't require any external libraries)
    static bool writeBMP(const std::string& filename, int width, int height, const unsigned char* data) {
        std::ofstream file(filename, std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        // BMP header (14 bytes)
        const unsigned char bmpHeader[14] = {
            'B', 'M',                          // signature
            0, 0, 0, 0,                        // file size (filled later)
            0, 0, 0, 0,                        // reserved
            54, 0, 0, 0                        // pixel data offset
        };

        // DIB header (40 bytes)
        const unsigned char dibHeader[40] = {
            40, 0, 0, 0,                       // DIB header size
            0, 0, 0, 0,                        // width (filled later)
            0, 0, 0, 0,                        // height (filled later)
            1, 0,                              // color planes
            24, 0,                             // bits per pixel (RGB)
            0, 0, 0, 0,                        // compression (none)
            0, 0, 0, 0,                        // image size (filled later)
            0, 0, 0, 0,                        // horizontal resolution (pixels/meter)
            0, 0, 0, 0,                        // vertical resolution (pixels/meter)
            0, 0, 0, 0,                        // colors in palette
            0, 0, 0, 0                         // important colors
        };

        // Calculate row size and padding
        int rowSize = width * 3;
        int padding = (4 - (rowSize % 4)) % 4;
        int imageSize = (rowSize + padding) * height;
        int fileSize = 54 + imageSize;

        // Fill in the headers with calculated values
        unsigned char header[54];
        std::memcpy(header, bmpHeader, 14);
        std::memcpy(header + 14, dibHeader, 40);

        // File size
        header[2] = (unsigned char)(fileSize);
        header[3] = (unsigned char)(fileSize >> 8);
        header[4] = (unsigned char)(fileSize >> 16);
        header[5] = (unsigned char)(fileSize >> 24);

        // Width
        header[18] = (unsigned char)(width);
        header[19] = (unsigned char)(width >> 8);
        header[20] = (unsigned char)(width >> 16);
        header[21] = (unsigned char)(width >> 24);

        // Height
        header[22] = (unsigned char)(height);
        header[23] = (unsigned char)(height >> 8);
        header[24] = (unsigned char)(height >> 16);
        header[25] = (unsigned char)(height >> 24);

        // Image size
        header[34] = (unsigned char)(imageSize);
        header[35] = (unsigned char)(imageSize >> 8);
        header[36] = (unsigned char)(imageSize >> 16);
        header[37] = (unsigned char)(imageSize >> 24);

        // Write the headers
        file.write(reinterpret_cast<const char*>(header), 54);

        // Write the pixel data
        std::vector<unsigned char> row(rowSize + padding, 0);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                // Convert RGBA to BGR (BMP format)
                int pixelIndex = (y * width + x) * 4;
                int rowIndex = x * 3;
                row[rowIndex]     = data[pixelIndex + 2]; // B
                row[rowIndex + 1] = data[pixelIndex + 1]; // G
                row[rowIndex + 2] = data[pixelIndex];     // R
                // Alpha is ignored in BMP
            }
            file.write(reinterpret_cast<const char*>(row.data()), rowSize + padding);
        }

        file.close();
        std::cout << "Screenshot saved to: " << filename << std::endl;
        return true;
    }
};

} // namespace VoxelEngine 