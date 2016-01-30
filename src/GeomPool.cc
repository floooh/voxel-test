//------------------------------------------------------------------------------
//  GeomPool.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Gfx/Gfx.h"
#include "glm/geometric.hpp"

#define STB_VOXEL_RENDER_IMPLEMENTATION
#include "GeomPool.h"

using namespace Oryol;

//------------------------------------------------------------------------------
void
GeomPool::Setup(const GfxSetup& gfxSetup) {

    // setup the stbvox mesh maker
    stbvox_init_mesh_maker(&this->meshMaker);
    stbvox_set_default_mesh(&this->meshMaker, 0);

    // setup a static index mesh which is shared by all geom meshes
    uint16 indices[Geom::MaxNumIndices];
    for (int quadIndex = 0; quadIndex < Geom::MaxNumQuads; quadIndex++) {
        int baseVertexIndex = quadIndex * 4;
        int ii = quadIndex * 6;
        indices[ii]   = baseVertexIndex + 0;
        indices[ii+1] = baseVertexIndex + 1;
        indices[ii+2] = baseVertexIndex + 2;
        indices[ii+3] = baseVertexIndex + 0;
        indices[ii+4] = baseVertexIndex + 2;
        indices[ii+5] = baseVertexIndex + 3;
    }
    auto meshSetup = MeshSetup::FromData(Usage::InvalidUsage, Usage::Immutable);
    meshSetup.NumVertices = 0;
    meshSetup.NumIndices  = Geom::MaxNumIndices;
    meshSetup.IndicesType = IndexType::Index16;
    meshSetup.PrimType = PrimitiveType::Triangles;
    meshSetup.DataVertexOffset = InvalidIndex;
    meshSetup.DataIndexOffset = 0;
    this->indexMesh = Gfx::CreateResource(meshSetup, indices, sizeof(indices));

    // setup the geoms
    for (auto& geom : this->geoms) {
        geom.Setup(gfxSetup, this->indexMesh);
    }
    this->Reset();
}

//------------------------------------------------------------------------------
void
GeomPool::Discard() {
    this->indexMesh.Invalidate();
    this->freeGeoms.Clear();
    this->validGeoms.Clear();
    for (auto& geom : this->geoms) {
        geom.Discard();
    }
}

//------------------------------------------------------------------------------
void
GeomPool::Reset() {
    this->freeGeoms.Clear();
    this->validGeoms.Clear();
    for (int i = 0; i < NumGeoms; i++) {
        this->freeGeoms.Add(i);
    }
}

//------------------------------------------------------------------------------
void
GeomPool::Begin(const glm::mat4& view_, const glm::mat4& proj_, const glm::vec3& lightDir_) {
    this->view = view_;
    this->proj = proj_;
    this->lightDir = lightDir_;
    stbvox_reset_buffers(&this->meshMaker);
    stbvox_set_buffer(&this->meshMaker, 0, 0, this->vertices, sizeof(this->vertices));
}

//------------------------------------------------------------------------------
void
GeomPool::Meshify(const Volume& vol) {
    const int strideX = vol.ArraySize.y * vol.ArraySize.z;
    const int strideY = vol.ArraySize.z;

    stbvox_set_input_stride(&this->meshMaker, strideX, strideY);
    stbvox_set_input_range(&this->meshMaker,
        vol.Offset.x, vol.Offset.y, vol.Offset.z,
        vol.Offset.x + vol.Size.x,
        vol.Offset.y + vol.Size.y,
        vol.Offset.z + vol.Size.z);
    stbvox_input_description* desc = stbvox_get_input_description(&this->meshMaker);
    desc->blocktype = vol.Blocks;
    desc->rgb = (stbvox_rgb*) vol.Colors;

    while (0 == stbvox_make_mesh(&this->meshMaker)) {
        this->bakeGeom();
    }
}

//------------------------------------------------------------------------------
void
GeomPool::End() {
    this->bakeGeom();
}

//------------------------------------------------------------------------------
void
GeomPool::bakeGeom() {
    int geomIndex = this->freeGeoms.PopBack();
    Geom& geom = this->geoms[geomIndex];
    geom.NumQuads = stbvox_get_quad_count(&this->meshMaker, 0);
    if (geom.NumQuads > 0) {

        // copy vertices into vertex buffer
        Gfx::UpdateVertices(geom.Mesh, this->vertices, geom.NumQuads*4*sizeof(Vertex));

        // update shader params
        float transform[3][3];
        stbvox_get_transform(&this->meshMaker, transform);
        geom.VSParams.Scale.x = transform[0][0];
        geom.VSParams.Scale.y = transform[0][1];
        geom.VSParams.Scale.z = transform[0][2];
        geom.VSParams.Translate.x = transform[1][0];
        geom.VSParams.Translate.y = transform[1][1];
        geom.VSParams.Translate.z = transform[1][2];
        geom.VSParams.TexTranslate.x = transform[2][0];
        geom.VSParams.TexTranslate.y = transform[2][1];
        geom.VSParams.TexTranslate.z = transform[2][2];
        const glm::mat4 model = glm::mat4();
        geom.VSParams.ModelViewProjection = this->proj * this->view * model;
        geom.VSParams.Model = model;
        geom.VSParams.LightDir = this->lightDir;

        // add geom to valid list
        this->validGeoms.Add(geomIndex);
    }
    else {
        this->freeGeoms.Add(geomIndex);
    }

    // reset for next run
    stbvox_reset_buffers(&this->meshMaker);
    stbvox_set_buffer(&this->meshMaker, 0, 0, this->vertices, sizeof(this->vertices));
}
