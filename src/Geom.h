#pragma once
//------------------------------------------------------------------------------
/**
    @class Geom
    @brief a 'meshified' voxel chunk in a geometry pool
*/
#include "Core/Types.h"
#include "Gfx/Gfx.h"
#include "glm/mat4x4.hpp"
#include "shaders.h"

class Geom {
public:
    static const int MaxNumVertices = (1<<15);
    static const int MaxNumQuads = MaxNumVertices / 4;
    static const int MaxNumIndices = MaxNumQuads * 6;

    /// one-time init
    void Setup(const Oryol::GfxSetup& gfxSetup, Oryol::Id indexMesh, Oryol::Id shd, const Oryol::Shaders::Voxel::VSParams& params);
    /// one-time discard
    void Discard();

    int NumQuads = 0;
    Oryol::Id Mesh;
    Oryol::Id DrawState;
    Oryol::Shaders::Voxel::VSParams VSParams;
};
