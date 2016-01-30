#pragma once
//------------------------------------------------------------------------------
/**
    @class GeomPool
    @brief a pool of reusable voxel meshes
*/
#include "Geom.h"
#include "Volume.h"
#include "Core/Containers/StaticArray.h"
#include "Core/Containers/Array.h"
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

#define STBVOX_CONFIG_MODE (30)
#include "stb_voxel_render.h"

class GeomPool {
public:
    static const int NumGeoms = 64;

    /// initialize the geom pool
    void Setup(const Oryol::GfxSetup& gfxSetup);
    /// discard the geom pool
    void Discard();

    /// put all geoms into the free bucket
    void Reset();
    /// begin meshifying
    void Begin(const glm::mat4& view, const glm::mat4& proj, const glm::vec3& lightDir);
    /// meshify a volume, takes free geoms, produces valid geoms
    void Meshify(const Volume& vol);
    /// end meshifying
    void End();

    /// bake a new mesh geom
    void bakeGeom();

    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 lightDir;

    Oryol::Id indexMesh;
    Oryol::StaticArray<Geom, NumGeoms> geoms;
    Oryol::Array<int> freeGeoms;                // indices of geoms that are currently free for meshing
    Oryol::Array<int> validGeoms;               // indices of geoms that are available for drawing

    stbvox_mesh_maker meshMaker;

    // temp target buffer for voxel meshing
    struct Vertex {
        Oryol::uint32 attr_vertex = 0;
        Oryol::uint32 attr_face = 0;
    } vertices[Geom::MaxNumVertices];
};