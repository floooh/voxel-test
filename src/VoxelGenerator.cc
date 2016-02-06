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
VoxelGenerator::Gen(int x0, int x1, int y0, int y1) {

    o_assert_dbg((x1 > x0) && (y1 > y0));

    Volume vol;
    vol.Blocks = (uint8*) this->voxels;
    vol.ArraySizeX = vol.ArraySizeY = VolumeSizeXY;
    vol.ArraySizeZ = VolumeSizeZ;
    vol.SizeX = vol.SizeY = Config::ChunkSizeXY;
    vol.SizeZ = Config::ChunkSizeZ;
    vol.OffsetX = vol.OffsetY = vol.OffsetZ = 1;

    const float voxelSizeX = (x1-x0)/float(Config::ChunkSizeXY);
    const float voxelSizeY = (y1-y0)/float(Config::ChunkSizeXY);

    glm::vec2 p;
    p.x = (x0-voxelSizeX) / float(Config::MapDimVoxels);
    const float dx = ((x1-x0)+2*voxelSizeX) / float(Config::MapDimVoxels*VolumeSizeXY);
    const float dy = ((y1-y0)+2*voxelSizeY) / float(Config::MapDimVoxels*VolumeSizeXY);
    for (int x = 0; x < VolumeSizeXY; x++, p.x+=dx) {
        p.y = (y0-voxelSizeY) / float(Config::MapDimVoxels);
        for (int y = 0; y < VolumeSizeXY; y++, p.y+=dy) {
            float n = glm::simplex(p*2.0f);
            n += glm::simplex(p*7.0f)*0.5f;
            n += glm::simplex(p*20.0f)*0.25f;
            int8 ni = glm::clamp(n*0.5f + 0.5f, 0.0f, 1.0f) * 31;
            this->voxels[x][y][0] = 1;
            for (int z = 1; z < VolumeSizeZ; z++) {
                this->voxels[x][y][z] = z < ni ? z:0;
            }
        }
    }
    return vol;
}
