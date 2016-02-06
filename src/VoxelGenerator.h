#pragma once
//------------------------------------------------------------------------------
/**
    @VoxelGenerator
    @brief generate voxel chunk data and meshify them
*/
#include "Volume.h"
#include "Config.h"

class VoxelGenerator {
public:
    static const int VolumeSizeXY = Config::ChunkSizeXY + 2;
    static const int VolumeSizeZ = Config::ChunkSizeZ + 2;

    /// generate voxel data
    Volume Gen(int x0, int x1, int y0, int y1);

    Oryol::uint8 voxels[VolumeSizeXY][VolumeSizeXY][VolumeSizeZ];
};