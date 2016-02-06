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
void
VoxelGenerator::Setup() {

    // seamless noise: http://www.gamedev.net/blog/33/entry-2138456-seamless-noise/
    const float size = SimplexSize;
    const float pi2 = 2.0f * glm::pi<float>();
    const float d = 1.0f / size;
    float s = 0.0f;
    for (int x = 0; x < SimplexSize; x++, s+=d) {
        const float s_pi2 = s*pi2;
        const float nx = glm::cos(s_pi2);
        const float nz = glm::sin(s_pi2);
        float t = 0.0f;
        for (int y = 0; y < SimplexSize; y++, t+=d) {
            const float t_pi2 = t*pi2;
            float ny = glm::cos(t_pi2);
            float nw = glm::sin(t_pi2);
            float n = glm::simplex(glm::vec4(nx,ny,nz,nw));
            int8 in = int8(n * 128);
            this->simplex[x][y] = in;
        }
    }
}

//------------------------------------------------------------------------------
Volume
VoxelGenerator::Gen(int x0, int y0) {

    Volume vol;
    vol.Blocks = (uint8*) this->voxels;
    vol.ArraySizeX = vol.ArraySizeY = VolumeSizeXY;
    vol.ArraySizeZ = VolumeSizeZ;
    vol.SizeX = vol.SizeY = ChunkSizeXY;
    vol.SizeZ = ChunkSizeZ;
    vol.OffsetX = vol.OffsetY = vol.OffsetZ = 1;
    const int mask = SimplexSize-1;
    for (int x = 0; x < VolumeSizeXY; x++) {
        const int xn = (x0 + x);
        for (int y = 0; y < VolumeSizeXY; y++) {
            const int yn = (y0 + y);
            int n = this->simplex[xn&mask][yn&mask];
            n += this->simplex[(xn*3)&mask][(yn*3)&mask] / 2;
            n = (n + 128) >> 3;
            if (n < 0) n = 0;
            else if (n > 31) n = 31;

            this->voxels[x][y][0] = 1;
            for (int z = 1; z < VolumeSizeZ; z++) {
                this->voxels[x][y][z] = z < n ? z:0;
            }
        }
    }
    return vol;
}
