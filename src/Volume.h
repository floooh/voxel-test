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

    int ArraySizeX = 0;
    int ArraySizeY = 0;
    int ArraySizeZ = 0;

    int OffsetX = 0;
    int OffsetY = 0;
    int OffsetZ = 0;

    int SizeX = 0;
    int SizeY = 0;
    int SizeZ = 0;

    int TranslationX = 0;
    int TranslationY = 0;
    int TranslationZ = 0;

    Oryol::int8 MinZ = 127;
    Oryol::int8 MaxZ = -127;
};
