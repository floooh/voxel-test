#pragma once
//------------------------------------------------------------------------------
/**
    @class Volume
    @brief a chunk of voxels in a big 3D array
*/
#include "Core/Types.h"
#include "glm/vec3.hpp"

class Volume {
public:
    // start pointers to block types and colors
    Oryol::uint8* Blocks = nullptr;
    Oryol::uint8* Colors = nullptr;

    glm::ivec3 ArraySize;   // size of of 3D array (world)
    glm::ivec3 Offset;      // start offset of this volume in 3D array
    glm::ivec3 Size;        // size of this volume in 3D array
    glm::ivec3 Translation; // world-space translation of this volume chunk
};
