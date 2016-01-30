//------------------------------------------------------------------------------
//  Geom.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Geom.h"

using namespace Oryol;

//------------------------------------------------------------------------------
void
Geom::Setup(const GfxSetup& gfxSetup, Id indexMesh) {

    // static shader params
    // FIXME: better move those into a separate uniform block?
    this->VSParams.NormalTable[0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    this->VSParams.NormalTable[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    this->VSParams.NormalTable[2] = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
    this->VSParams.NormalTable[3] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    this->VSParams.NormalTable[4] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    this->VSParams.NormalTable[5] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    // create Gfx resources
    auto meshSetup = MeshSetup::Empty(MaxNumVertices, Usage::Dynamic);
    meshSetup.Layout
        .Add(VertexAttr::Position, VertexFormat::UByte4)
        .Add(VertexAttr::Normal, VertexFormat::UByte4);
    this->Mesh = Gfx::CreateResource(meshSetup);
    Id shd = Gfx::CreateResource(Shaders::Voxel::Setup());
    auto dss = DrawStateSetup::FromMeshAndShader(indexMesh, shd);
    dss.Meshes[1] = this->Mesh;
    dss.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    dss.DepthStencilState.DepthWriteEnabled = true;
    dss.RasterizerState.CullFaceEnabled = true;
    dss.RasterizerState.CullFace = Face::Front;
    dss.RasterizerState.SampleCount = gfxSetup.SampleCount;
    this->DrawState = Gfx::CreateResource(dss);
}

//------------------------------------------------------------------------------
void
Geom::Discard() {
    this->Mesh.Invalidate();
    this->DrawState.Invalidate();
}
