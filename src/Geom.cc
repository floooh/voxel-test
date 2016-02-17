//------------------------------------------------------------------------------
//  Geom.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Geom.h"

using namespace Oryol;

//------------------------------------------------------------------------------
void
Geom::Setup(const GfxSetup& gfxSetup, Id indexMesh, Id shd, const Shaders::Voxel::VSParams& params) {

    // static shader params
    // FIXME: better move those into a separate uniform block?
    this->VSParams = params;

    // create Gfx resources
    auto meshSetup = MeshSetup::Empty(MaxNumVertices, Usage::Dynamic);
    meshSetup.Layout
        .Add(VertexAttr::Position, VertexFormat::UByte4)
        .Add(VertexAttr::Normal, VertexFormat::UByte4);
    this->Mesh = Gfx::CreateResource(meshSetup);
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