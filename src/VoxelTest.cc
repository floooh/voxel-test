//------------------------------------------------------------------------------
//  VoxelTest.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/App.h"
#include "Gfx/Gfx.h"
#include "glm/vec4.hpp"
#include "shaders.h"

#define STBVOX_CONFIG_MODE (30)
#define STB_VOXEL_RENDER_IMPLEMENTATION
#include "stb_voxel_render.h"

using namespace Oryol;

class VoxelTest : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();

    void init_stbvox();
    void init_volume();
    void init_mesh_data();
    void init_resources();

    stbvox_mesh_maker meshMaker;

    ClearState clearState;
    Id mesh;
    Id drawState;

    static const int SizeX = 8;
    static const int SizeY = 8;
    static const int SizeZ = 8;
    uchar volume[SizeX][SizeY][SizeZ];

    static const int MaxNumVertices = (1<<16);
    struct Vertex {
        uint32 attr_vertex = 0;
        uint32 attr_face = 0;
    } vertices[MaxNumVertices];
};
OryolMain(VoxelTest);

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnInit() {
    Gfx::Setup(GfxSetup::Window(800, 600, "Oryol Voxel Test"));
    this->clearState = ClearState::ClearAll(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 1.0f, 0);

    this->init_stbvox();
    this->init_volume();
    this->init_mesh_data();
    this->init_resources();

    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnRunning() {
    Gfx::ApplyDefaultRenderTarget(this->clearState);
    Gfx::CommitFrame();
    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnCleanup() {
    Gfx::Discard();
    return App::OnCleanup();
}

//------------------------------------------------------------------------------
void
VoxelTest::init_stbvox() {
    stbvox_init_mesh_maker(&this->meshMaker);
    stbvox_set_buffer(&this->meshMaker, 0, 0, this->vertices, sizeof(this->vertices));
    stbvox_set_default_mesh(&this->meshMaker, 0);
    stbvox_set_input_stride(&this->meshMaker, SizeY*SizeZ, SizeZ);
    stbvox_set_input_range(&this->meshMaker, 1, 1, 1, SizeX-1, SizeY-1, SizeZ-1);

    stbvox_input_description* desc = stbvox_get_input_description(&this->meshMaker);
    desc->blocktype = &(this->volume[0][0][0]);
}

//------------------------------------------------------------------------------
void
VoxelTest::init_volume() {
    Memory::Clear(this->volume, sizeof(this->volume));
    this->volume[3][3][3] = 1;
    this->volume[4][4][4] = 1;
    this->volume[5][5][5] = 1;
}

//------------------------------------------------------------------------------
void
VoxelTest::init_mesh_data() {
    stbvox_make_mesh(&this->meshMaker);
    int numQuads = stbvox_get_quad_count(&this->meshMaker, 0);

    // dump generated data
    for (int quad = 0; quad < numQuads; quad++) {
        Log::Info("quad %d\n", quad);
        for (int corner = 0; corner < 4; corner++) {
            Log::Info("  corner %d\n", corner);
            int vtx = quad * 4 + corner;
            const uint32 attr_vertex = this->vertices[vtx].attr_vertex;
            const uint32 attr_face = this->vertices[vtx].attr_face;
            float offset_x = float(attr_vertex & 255);
            float offset_y = float((attr_vertex >> 8) & 255);
            float offset_z = float((attr_vertex >> 16) & 255);
            float amb_occ  = float((attr_vertex >> 24) & 63) / 63.0f;
            Log::Info("    offset  = %.3f,%.3f,%.3f\n"
                      "    amb_occ = %f\n"
                      "    facedata = 0x%08x\n",
                      offset_x, offset_y, offset_z,
                      amb_occ, attr_face);
        }
    }
}

//------------------------------------------------------------------------------
void
VoxelTest::init_resources() {

    // dynamic vertex buffer and static index buffer
    static const int numQuads = (1<<16) / 4;
    static const int numVertices = numQuads * 4;
    static const int numIndices = numQuads * 6;
    auto meshSetup = MeshSetup::FromData(Usage::Dynamic, Usage::Immutable);
    meshSetup.Layout
        .Add(VertexAttr::Position, VertexFormat::UByte4)
        .Add(VertexAttr::Normal, VertexFormat::UByte4);
    meshSetup.NumVertices = numVertices;
    meshSetup.NumIndices  = numIndices;
    meshSetup.IndicesType = IndexType::Index16;
    meshSetup.PrimType = PrimitiveType::Triangles;
    meshSetup.DataVertexOffset = InvalidIndex;
    meshSetup.DataIndexOffset = 0;

    uint16 indices[numIndices];
    for (int i = 0; i < numQuads; i++) {
        int baseVertexIndex = i * 4;
        indices[i]   = baseVertexIndex + 0;
        indices[i+1] = baseVertexIndex + 1;
        indices[i+2] = baseVertexIndex + 2;
        indices[i+3] = baseVertexIndex + 0;
        indices[i+4] = baseVertexIndex + 2;
        indices[i+5] = baseVertexIndex + 3;
    }
    this->mesh = Gfx::CreateResource(meshSetup, indices, sizeof(indices));

    // shader and drawstate
    Id shd = Gfx::CreateResource(Shaders::Voxel::Setup());
    auto dss = DrawStateSetup::FromMeshAndShader(this->mesh, shd);
    dss.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    dss.DepthStencilState.DepthWriteEnabled = true;
    dss.RasterizerState.CullFaceEnabled = true;
    this->drawState = Gfx::CreateResource(dss);
}
