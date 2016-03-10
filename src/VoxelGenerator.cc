//------------------------------------------------------------------------------
//  VoxelGenerator.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/Assertion.h"
#include "glm/vec2.hpp"
#include "glm/gtc/noise.hpp"
#include "glm/gtc/constants.hpp"
#include "glm/trigonometric.hpp"
#include "Core/Memory/Memory.h"
#include "VoxelGenerator.h"
#include "Volume.h"
#include <algorithm>

using namespace Oryol;

//------------------------------------------------------------------------------
Volume
VoxelGenerator::initVolume() {
    Volume vol;
    vol.Blocks = (uint8*) this->voxels;
    vol.ArraySizeX = vol.ArraySizeY = VolumeSizeXY;
    vol.ArraySizeZ = VolumeSizeZ;
    vol.SizeX = vol.SizeY = Config::ChunkSizeXY;
    vol.SizeZ = Config::ChunkSizeZ;
    vol.OffsetX = vol.OffsetY = vol.OffsetZ = 1;
    return vol;
}

//------------------------------------------------------------------------------
Volume
VoxelGenerator::GenSimplex(const VisBounds& bounds) {

    const int x0 = bounds.x0;
    const int x1 = bounds.x1;
    const int y0 = bounds.y0;
    const int y1 = bounds.y1;

    const float voxelSizeX = (x1-x0)/float(Config::ChunkSizeXY);
    const float voxelSizeY = (y1-y0)/float(Config::ChunkSizeXY);

    Volume vol = this->initVolume();
    glm::vec2 p;
    p.x = (x0-(voxelSizeX*0.5f)) / float(Config::MapDimVoxels);
    const float dx = ((x1-x0)+2*voxelSizeX) / float(Config::MapDimVoxels*VolumeSizeXY);
    const float dy = ((y1-y0)+2*voxelSizeY) / float(Config::MapDimVoxels*VolumeSizeXY);
    for (int x = 0; x < VolumeSizeXY; x++, p.x+=dx) {
        p.y = (y0-(voxelSizeY*0.5f)) / float(Config::MapDimVoxels);
        for (int y = 0; y < VolumeSizeXY; y++, p.y+=dy) {
            float n = glm::simplex(p*0.5f) * 1.5f;
            n += glm::simplex(p*2.5f)*0.35f;
            n += glm::simplex(p*10.0f)*0.55f;
            int8 ni = glm::clamp(n*0.5f + 0.5f, 0.0f, 1.0f) * 31;
            this->voxels[x][y][0] = 1;
            for (int z = 1; z < VolumeSizeZ; z++) {
                this->voxels[x][y][z] = z < ni ? z:0;
            }
        }
    }
    return vol;
}

//------------------------------------------------------------------------------
Volume
VoxelGenerator::GenDebug(const VisBounds& bounds, int lvl) {
    int8 blockType = lvl+1;
    Volume vol = this->initVolume();
    Memory::Clear(this->voxels, sizeof(this->voxels));
    for (int x = 0; x < VolumeSizeXY; x++) {
        for (int y = 0; y < VolumeSizeXY; y++) {
            int8 bt = blockType;
            if ((x<=1)||(y<=1)||(x>=VolumeSizeXY-2)||(y>=VolumeSizeXY-2)) {
                bt = blockType + 1;
            }
            this->voxels[x][y][lvl+1] = bt;
        }
    }
    return vol;
}

//------------------------------------------------------------------------------
void
VoxelGenerator::createCirclePlane(const glm::ivec3& center, int width, int depth, double radius, Volume& volume) {
    const double xRadius = (width - 1) / 2.0;
    const double yRadius = (depth - 1) / 2.0;
    const double minRadius = std::min(xRadius, yRadius);
    const double xRatio = xRadius / (float) minRadius;
    const double zRatio = yRadius / (float) minRadius;

    for (double y = -yRadius; y <= yRadius; ++y) {
        for (double x = -xRadius; x <= xRadius; ++x) {
            const double xP = x / xRatio;
            const double zP = y / zRatio;
            const double distance = sqrt(pow(xP, 2) + pow(zP, 2));
            if (distance < radius) {
                const int _x = center.x + x;
                const int _y = center.y + y;
                const int _z = center.z;
                if (_x < VolumeSizeXY && _z < VolumeSizeZ) {
                    this->voxels[_x][_y][_z] = 1;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void
VoxelGenerator::createCube(const glm::ivec3& pos, int width, int height, int depth, Volume& volume) {
    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            for (int z = 0; z < depth; ++z) {
                const int _x = pos.x + x;
                const int _y = pos.y + y;
                const int _z = pos.z + z;
                if (_x < VolumeSizeXY && _y < VolumeSizeXY && _z < VolumeSizeZ) {
                    this->voxels[_x][_y][_z] = 1;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void
VoxelGenerator::createPlane(const glm::ivec3& pos, int width, int depth, Volume& volume) {
    createCube(pos, width, 1, depth, volume);
}

//------------------------------------------------------------------------------
void
VoxelGenerator::createEllipse(const glm::ivec3& pos, int width, int height, int depth, Volume& volume) {
    const double heightRadius = (height - 1.0) / 2.0;
    const double minDimension = std::min(width, depth);
    const double adjustedMinRadius = (minDimension - 1.0) / 2.0;
    const double heightFactor = heightRadius / adjustedMinRadius;
    for (double z = -heightRadius; z <= heightRadius; ++z) {
        const double adjustedHeight = abs(z / heightFactor);
        const double circleRadius = sqrt(pow(adjustedMinRadius + 0.5, 2.0) - pow(adjustedHeight, 2.0));
        const glm::ivec3 planePos(pos.x, pos.y, pos.z + z);
        createCirclePlane(planePos, width, depth, circleRadius, volume);
    }
}

//------------------------------------------------------------------------------
void
VoxelGenerator::createCone(const glm::ivec3& pos, int width, int height, int depth, Volume& volume) {
    const double heightRadius = height - 0.5;
    const double minDimension = std::min(width, depth);
    const double minRadius = minDimension / 2.0;
    for (double z = 0.5; z <= heightRadius; z++) {
        const double percent = 1 - (z / height);
        const double circleRadius = percent * minRadius;
        const glm::ivec3 planePos(pos.x, pos.y, pos.z + z);
        createCirclePlane(planePos, width, depth, circleRadius, volume);
    }
}

//------------------------------------------------------------------------------
void
VoxelGenerator::createDome(const glm::ivec3& pos, int width, int height, int depth, Volume& volume) {
    // TODO:
}

//------------------------------------------------------------------------------
void
VoxelGenerator::addTree(const glm::ivec3& pos, TreeType type, int trunkHeight, Volume& volume) {
    const int top = (int) pos.z + trunkHeight;
    const int sizeX = VolumeSizeXY;
    const int sizeY = VolumeSizeXY;
    const int sizeZ = VolumeSizeZ;

    Log::Dbg("generate tree at %i:%i:%i\n", pos.x, pos.z, pos.y);

    for (int z = pos.z; z < top; ++z) {
        const int width = std::max(1, 3 - (z - pos.z));
        for (int x = pos.x - width; x < pos.x + width; ++x) {
            for (int y = pos.y - width; y < pos.y + width; ++y) {
                if ((x >= pos.x + 1 || x < pos.x - 1) && (y >= pos.y + 1 || y < pos.y - 1))
                    continue;
                if (x < 0 || z < 0 || y < 0 || x >= sizeX || z >= sizeY || y >= sizeZ)
                    continue;

                this->voxels[x][y][z] = 1;
            }
        }
    }

    const int width = 16;
    const int depth = 16;
    const int height = 12;
    if (type == TreeType::ELLIPSIS) {
        const int centerLeavesPos = top + height / 2;
        const glm::ivec3 leafesPos(pos.x, pos.y, centerLeavesPos);
        createEllipse(leafesPos, width, height, depth, volume);
    } else if (type == TreeType::CONE) {
        const glm::ivec3 leafesPos(pos.x, pos.y, top);
        createCone(leafesPos, width, height, depth, volume);
    } else if (type == TreeType::DOME) {
        const glm::ivec3 leafesPos(pos.x, pos.y, top);
        createDome(leafesPos, width, height, depth, volume);
    }
}

//------------------------------------------------------------------------------
void
VoxelGenerator::createTrees(Volume& volume) {
    Log::Dbg("generate trees\n");
    glm::ivec3 pos(volume.SizeX / 2, volume.SizeY / 2, -1);
    for (int i = VolumeSizeZ - 1; i > 0; --i) {
        if (this->voxels[pos.x][pos.y][i] != 0) {
            pos.z = i + 1;
            break;
        }
    }
    if (pos.z == -1)
        return;
    addTree(pos, TreeType::CONE, 6, volume);
}

//------------------------------------------------------------------------------
void
VoxelGenerator::createClouds(Volume& volume) {
#if 0
    Log::Dbg("generate clouds\n");
    const int height = 10;
    glm::ivec3 pos(volume.SizeX / 2, volume.SizeY / 2, VolumeSizeZ - 3);
    createEllipse(pos, 10, height, 10, volume);
    pos.x -= 5;
    pos.y -= 5;
    createEllipse(pos, 20, height, 35, volume);
#endif
}
