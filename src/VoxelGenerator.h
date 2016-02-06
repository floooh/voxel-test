#pragma once
//------------------------------------------------------------------------------
/**
    @VoxelGenerator
    @brief generate voxel chunk data and meshify them
*/
#include "Volume.h"

class VoxelGenerator {
public:
    static const int ChunkSizeXY = 254;
    static const int ChunkSizeZ = 32;
    static const int VolumeSizeXY = ChunkSizeXY + 2;
    static const int VolumeSizeZ = VolumeSizeXY + 2;
    static const int SimplexSize = 256;

    /// populate the simplex noise lookup table
    void Setup();
    /// generate voxel data
    Volume Gen(int x, int y);

    Oryol::uint8 voxels[VolumeSizeXY][VolumeSizeXY][VolumeSizeZ];
    Oryol::int8 simplex[SimplexSize][SimplexSize];
};