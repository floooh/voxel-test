#pragma once
//------------------------------------------------------------------------------
/**
    @VoxelGenerator
    @brief generate voxel chunk data and meshify them
*/
#include "Volume.h"
#include "Config.h"
#include "VisBounds.h"

class VoxelGenerator {
public:
    static const int VolumeSizeXY = Config::ChunkSizeXY + 2;
    static const int VolumeSizeZ = Config::ChunkSizeZ + 2;

    enum class TreeType {
        DOME, CONE, ELLIPSIS
    };

    void createCirclePlane(const glm::ivec3& center, int width, int depth, double radius, Volume& volume);
    void createCube(const glm::ivec3& pos, int width, int height, int depth, Volume& volume);
    void createPlane(const glm::ivec3& pos, int width, int depth, Volume& volume);
    void createEllipse(const glm::ivec3& pos, int width, int height, int depth, Volume& volume);
    void createCone(const glm::ivec3& pos, int width, int height, int depth, Volume& volume);
    void createDome(const glm::ivec3& pos, int width, int height, int depth, Volume& volume);
    void addTree(const glm::ivec3& pos, TreeType type, int trunkHeight, Volume& volume);
    void createClouds(Volume& volume);
    void createTrees(Volume& volume);

    /// generate simplex noise voxel data
    Volume GenSimplex(const VisBounds& bounds);
    /// generate debug voxel data
    Volume GenDebug(const VisBounds& bounds, int lvl);

    /// initialize a volume object
    Volume initVolume();

    Oryol::uint8 voxels[VolumeSizeXY][VolumeSizeXY][VolumeSizeZ];
};
