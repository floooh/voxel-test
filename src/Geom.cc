//------------------------------------------------------------------------------
//  Geom.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Geom.h"

using namespace Oryol;

//------------------------------------------------------------------------------
void
Geom::Setup(const GfxSetup& gfxSetup, 
            const VertexLayout& layout, 
            Id indexMesh, 
            Id shd, 
            const Shaders::Voxel::VSParams& params) {

    // static shader params
    // FIXME: better move those into a separate uniform block?
    this->VSParams = params;

    // create dynamically updated voxel mesh
    auto meshSetup = MeshSetup::Empty(MaxNumVertices, Usage::Dynamic);
    meshSetup.Layout = layout;
    this->Mesh = Gfx::CreateResource(meshSetup);
}

//------------------------------------------------------------------------------
void
Geom::Discard() {
    this->Mesh.Invalidate();
}
