#include "Chunk.hpp"

namespace VoxelEngine {

Chunk::Chunk(const glm::ivec3& position)
    : m_Position(position)
{
    // Initialize all voxels to air
    std::fill(m_Voxels.begin(), m_Voxels.end(), VoxelType::Air);
    
    // Create renderer
    m_Renderer = std::make_unique<VoxelRenderer>();
}

Chunk::~Chunk() {
    // Resources are automatically cleaned up by unique_ptr
}

void Chunk::setVoxel(int x, int y, int z, VoxelType type) {
    if (!isValidPosition(x, y, z)) {
        return;
    }
    
    int index = (y * CHUNK_SIZE_Z + z) * CHUNK_SIZE_X + x;
    m_Voxels[index] = type;
}

VoxelType Chunk::getVoxel(int x, int y, int z) const {
    if (!isValidPosition(x, y, z)) {
        return VoxelType::Air;
    }
    
    int index = (y * CHUNK_SIZE_Z + z) * CHUNK_SIZE_X + x;
    return m_Voxels[index];
}

bool Chunk::isValidPosition(int x, int y, int z) const {
    return x >= 0 && x < CHUNK_SIZE_X &&
           y >= 0 && y < CHUNK_SIZE_Y &&
           z >= 0 && z < CHUNK_SIZE_Z;
}

void Chunk::buildMesh() {
    m_Renderer->clear();
    
    // Loop through all voxels in the chunk
    for (int y = 0; y < CHUNK_SIZE_Y; y++) {
        for (int z = 0; z < CHUNK_SIZE_Z; z++) {
            for (int x = 0; x < CHUNK_SIZE_X; x++) {
                VoxelType type = getVoxel(x, y, z);
                
                // Skip air voxels
                if (type == VoxelType::Air) {
                    continue;
                }
                
                // Check each face
                if (shouldRenderFace(x, y, z, VoxelFace::Direction::Front)) {
                    VoxelFace face = { VoxelFace::Direction::Front, type, glm::ivec3(x, y, z) + m_Position * glm::ivec3(CHUNK_SIZE_X, 0, CHUNK_SIZE_Z) };
                    m_Renderer->addFace(face);
                }
                
                if (shouldRenderFace(x, y, z, VoxelFace::Direction::Back)) {
                    VoxelFace face = { VoxelFace::Direction::Back, type, glm::ivec3(x, y, z) + m_Position * glm::ivec3(CHUNK_SIZE_X, 0, CHUNK_SIZE_Z) };
                    m_Renderer->addFace(face);
                }
                
                if (shouldRenderFace(x, y, z, VoxelFace::Direction::Top)) {
                    VoxelFace face = { VoxelFace::Direction::Top, type, glm::ivec3(x, y, z) + m_Position * glm::ivec3(CHUNK_SIZE_X, 0, CHUNK_SIZE_Z) };
                    m_Renderer->addFace(face);
                }
                
                if (shouldRenderFace(x, y, z, VoxelFace::Direction::Bottom)) {
                    VoxelFace face = { VoxelFace::Direction::Bottom, type, glm::ivec3(x, y, z) + m_Position * glm::ivec3(CHUNK_SIZE_X, 0, CHUNK_SIZE_Z) };
                    m_Renderer->addFace(face);
                }
                
                if (shouldRenderFace(x, y, z, VoxelFace::Direction::Right)) {
                    VoxelFace face = { VoxelFace::Direction::Right, type, glm::ivec3(x, y, z) + m_Position * glm::ivec3(CHUNK_SIZE_X, 0, CHUNK_SIZE_Z) };
                    m_Renderer->addFace(face);
                }
                
                if (shouldRenderFace(x, y, z, VoxelFace::Direction::Left)) {
                    VoxelFace face = { VoxelFace::Direction::Left, type, glm::ivec3(x, y, z) + m_Position * glm::ivec3(CHUNK_SIZE_X, 0, CHUNK_SIZE_Z) };
                    m_Renderer->addFace(face);
                }
            }
        }
    }
    
    // Build the mesh
    m_Renderer->buildMesh();
}

void Chunk::render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    m_Renderer->render(viewMatrix, projectionMatrix);
}

void Chunk::setNeighbor(const glm::ivec3& direction, Chunk* chunk) {
    // Generate a unique key for the direction
    int key = (direction.x + 1) * 100 + (direction.y + 1) * 10 + (direction.z + 1);
    m_Neighbors[key] = chunk;
}

bool Chunk::shouldRenderFace(int x, int y, int z, VoxelFace::Direction direction) const {
    // Get the current voxel type
    VoxelType currentType = getVoxel(x, y, z);
    
    // If it's air, we don't need to render any faces
    if (currentType == VoxelType::Air) {
        return false;
    }
    
    int nx = x, ny = y, nz = z;
    
    // Get neighbor coords based on face direction
    switch (direction) {
        case VoxelFace::Direction::Front:  nz++; break;
        case VoxelFace::Direction::Back:   nz--; break;
        case VoxelFace::Direction::Top:    ny++; break;
        case VoxelFace::Direction::Bottom: ny--; break;
        case VoxelFace::Direction::Right:  nx++; break;
        case VoxelFace::Direction::Left:   nx--; break;
    }
    
    // Get the voxel type at the neighbor position
    VoxelType neighborType = getVoxelWorldSpace(nx, ny, nz);
    
    // Check if the neighbor is transparent or semi-transparent
    bool isNeighborTransparent = isVoxelTransparent(neighborType);
    
    // Special case for identical transparent blocks (like water next to water)
    if (isNeighborTransparent && currentType == neighborType) {
        // Don't render faces between identical transparent blocks
        return false;
    }
    
    // Render the face if the neighbor is transparent
    return isNeighborTransparent;
}

// Helper function to determine if a voxel type is transparent
bool Chunk::isVoxelTransparent(VoxelType type) const {
    // For debugging - uncomment to force all faces to render
    // return true; 
    
    switch (type) {
        case VoxelType::Air:
        case VoxelType::Water:
        case VoxelType::Leaves: // Semi-transparent
            return true;
        default:
            return false;
    }
}

VoxelType Chunk::getVoxelWorldSpace(int x, int y, int z) const {
    // If the position is within this chunk, use local lookup
    if (isValidPosition(x, y, z)) {
        return getVoxel(x, y, z);
    }
    
    // Position is outside this chunk
    // Calculate chunk relative coordinates
    int cx = 0, cy = 0, cz = 0;
    
    // X direction
    if (x < 0) {
        cx = -1;
        x += CHUNK_SIZE_X;
    } else if (x >= CHUNK_SIZE_X) {
        cx = 1;
        x -= CHUNK_SIZE_X;
    }
    
    // Y direction
    if (y < 0) {
        cy = -1;
        y += CHUNK_SIZE_Y;
    } else if (y >= CHUNK_SIZE_Y) {
        cy = 1;
        y -= CHUNK_SIZE_Y;
    }
    
    // Z direction
    if (z < 0) {
        cz = -1;
        z += CHUNK_SIZE_Z;
    } else if (z >= CHUNK_SIZE_Z) {
        cz = 1;
        z -= CHUNK_SIZE_Z;
    }
    
    // Generate a key for the neighbor chunk
    int key = (cx + 1) * 100 + (cy + 1) * 10 + (cz + 1);
    
    // Check if we have a neighbor in that direction
    auto it = m_Neighbors.find(key);
    if (it != m_Neighbors.end() && it->second != nullptr) {
        return it->second->getVoxel(x, y, z);
    }
    
    // No neighbor or invalid position, return air
    return VoxelType::Air;
}

} // namespace VoxelEngine 