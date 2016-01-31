//------------------------------------------------------------------------------
//  GeomPool.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "GeomPool.h"
#include "glm/geometric.hpp"

using namespace Oryol;

//------------------------------------------------------------------------------
void
GeomPool::Setup(const GfxSetup& gfxSetup) {

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
    this->freeGeoms.Reserve(NumGeoms);
    this->FreeAll();
}

//------------------------------------------------------------------------------
void
GeomPool::Discard() {
    this->indexMesh.Invalidate();
    this->freeGeoms.Clear();
    for (auto& geom : this->geoms) {
        geom.Discard();
    }
}

//------------------------------------------------------------------------------
void
GeomPool::FreeAll() {
    this->freeGeoms.Clear();
    for (int i = 0; i < NumGeoms; i++) {
        this->freeGeoms.Add(i);
    }
}
