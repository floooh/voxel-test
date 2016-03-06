//------------------------------------------------------------------------------
//  GeomPool.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "GeomPool.h"
#include "Config.h"
#include "Gfx/Gfx.h"
#include "glm/geometric.hpp"
#include "glm/gtc/random.hpp"

using namespace Oryol;

//------------------------------------------------------------------------------
void
GeomPool::Setup(const GfxSetup& gfxSetup) {

    // setup a static mesh with only indices which is shared by all geom meshes
    uint16 indices[Config::GeomMaxNumIndices];
    for (int quadIndex = 0; quadIndex < Config::GeomMaxNumQuads; quadIndex++) {
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
    meshSetup.NumIndices  = Config::GeomMaxNumIndices;
    meshSetup.IndicesType = IndexType::Index16;
    meshSetup.DataVertexOffset = InvalidIndex;
    meshSetup.DataIndexOffset = 0;
    this->IndexMesh = Gfx::CreateResource(meshSetup, indices, sizeof(indices));

    // setup shader params template
    Shaders::Voxel::VSParams vsParams;
    vsParams.NormalTable[0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    vsParams.NormalTable[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    vsParams.NormalTable[2] = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
    vsParams.NormalTable[3] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    vsParams.NormalTable[4] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    vsParams.NormalTable[5] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    for (int i = 0; i < int(sizeof(vsParams.ColorTable)/sizeof(glm::vec4)); i++) {
        vsParams.ColorTable[i] = glm::linearRand(glm::vec4(0.25f), glm::vec4(1.0f));
    }

    // setup shader and drawstate
    Id shd = Gfx::CreateResource(Shaders::Voxel::Setup());
    auto dss = DrawStateSetup::FromShader(shd);
    dss.Layouts[1]
        .Add(VertexAttr::Position, VertexFormat::UByte4N)
        .Add(VertexAttr::Normal, VertexFormat::UByte4N);
    dss.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    dss.DepthStencilState.DepthWriteEnabled = true;
    dss.RasterizerState.CullFaceEnabled = true;
    dss.RasterizerState.CullFace = Face::Front;
    dss.RasterizerState.SampleCount = gfxSetup.SampleCount;
    this->DrawState = Gfx::CreateResource(dss);

    // setup items
    meshSetup = MeshSetup::Empty(Config::GeomMaxNumVertices, Usage::Dynamic);
    meshSetup.Layout = dss.Layouts[1];
    for (auto& geom : this->Geoms) {
        geom.VSParams = vsParams;
        geom.NumQuads = 0;
        geom.Mesh = Gfx::CreateResource(meshSetup);
    }
    this->freeGeoms.Reserve(NumGeoms);
    this->FreeAll();
}

//------------------------------------------------------------------------------
void
GeomPool::Discard() {
    this->IndexMesh.Invalidate();
    this->DrawState.Invalidate();
    this->freeGeoms.Clear();
}

//------------------------------------------------------------------------------
void
GeomPool::FreeAll() {
    this->freeGeoms.Clear();
    for (int i = 0; i < NumGeoms; i++) {
        this->freeGeoms.Add(i);
    }
}
