#pragma once
//------------------------------------------------------------------------------
/**
    @class GeomMesher
    @brief meshify volumes into geoms
*/
#include "Geom.h"
#include "Volume.h"
#include "glm/vec3.hpp"

#define STBVOX_CONFIG_MODE (30)
#define STBVOX_CONFIG_PRECISION_Z (0)
#include "stb_voxel_render.h"

class GeomMesher {
public:
    /// result struct for Meshify method
    struct Result {
        bool VolumeDone = false;
        bool BufferFull = false;
        const void* Vertices = nullptr;
        int NumQuads = 0;
        int NumBytes = 0;
        glm::vec3 Scale;
        glm::vec3 Translate;
        glm::vec3 TexTranslate;
    };

    /// setup the geom mesher
    void Setup();
    /// discard the geom mesher
    void Discard();

    /// start meshifying, resets the stbox mesh maker
    void Start();
    /// start a new volume
    void StartVolume(const Volume& volume);
    /// do one meshify pass, continue to call until VolumeDone
    Result Meshify();

private:
    stbvox_mesh_maker meshMaker;
    struct vertex {
        Oryol::uint32 attr_vertex = 0;
        Oryol::uint32 attr_face = 0;
    } vertices[Geom::MaxNumVertices];
};