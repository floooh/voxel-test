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
    p.x = (x0-voxelSizeX) / float(Config::MapDimVoxels);
    const float dx = ((x1-x0)+2*voxelSizeX) / float(Config::MapDimVoxels*VolumeSizeXY);
    const float dy = ((y1-y0)+2*voxelSizeY) / float(Config::MapDimVoxels*VolumeSizeXY);
    for (int x = 0; x < VolumeSizeXY; x++, p.x+=dx) {
        p.y = (y0-voxelSizeY) / float(Config::MapDimVoxels);
        for (int y = 0; y < VolumeSizeXY; y++, p.y+=dy) {
            float n = glm::simplex(p*0.5f) * 1.5f;
            n += glm::simplex(p*2.5f)*0.35f;
            n += glm::simplex(p*10.0f)*0.55f;
            int8 ni = glm::clamp(n*0.5f + 0.5f, 0.0f, 1.0f) * 31;
            this->voxels[x][y][0] = 1;
            for (int z = 1; z < VolumeSizeZ; z++) {
                int8 h = z < ni ? z:0;
                if (h < vol.MinZ) {
                    vol.MinZ = h;
                }
                else if (h > vol.MaxZ) {
                    vol.MaxZ = h;
                }
                this->voxels[x][y][z] = h;
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
            if ((x<=1)||(y<=1)||(x>=VolumeSizeXY-2)||(y>=VolumeSizeXY-2)) {
                this->voxels[x][y][lvl+1] = 2*blockType;
            }
            else {
                this->voxels[x][y][lvl+1] = blockType;
            }
        }
    }
    return vol;
}