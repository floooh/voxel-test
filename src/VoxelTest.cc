//------------------------------------------------------------------------------
//  VoxelTest.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/App.h"
#include "Gfx/Gfx.h"
#include "Time/Clock.h"
#include "glm/vec4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/random.hpp"
#include "shaders.h"

#define STBVOX_CONFIG_MODE (30)
#define STB_VOXEL_RENDER_IMPLEMENTATION
#include "stb_voxel_render.h"

using namespace Oryol;

class Chunk {
public:
    void setup(const GfxSetup& gfxSetup, const glm::vec3& pos);
    void discard();
    void update_shader_params(const glm::mat4& view, const glm::mat4& proj);
    int32 update_volume(int32 updateIndex);
    void update_mesh();

    glm::vec3 pos;
    glm::mat4 model;
    Id mesh;
    Id drawState;
    Shaders::Voxel::VSParams vsParams;
    int numQuads = 0;

    stbvox_mesh_maker meshMaker;

    static const int SizeX = 16;
    static const int SizeY = 16;
    static const int SizeZ = 16;
    uchar blocks[SizeX][SizeY][SizeZ];
    stbvox_rgb colors[SizeX][SizeY][SizeZ];
    static const int MaxNumVertices = (1<<14);
    struct Vertex {
        uint32 attr_vertex = 0;
        uint32 attr_face = 0;
    } vertices[MaxNumVertices];
};

class VoxelTest : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();

    void update_camera();
    void upload_mesh_data();

    int32 frameIndex = 0;
    glm::mat4 view;
    glm::mat4 proj;

    ClearState clearState;
    static const int NumChunksX = 8;
    static const int NumChunksZ = 8;
    static const int NumChunks = NumChunksX * NumChunksZ;
    Chunk chunks[NumChunks];
};
OryolMain(VoxelTest);

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnInit() {
    auto gfxSetup = GfxSetup::WindowMSAA4(800, 600, "Oryol Voxel Test");
    Gfx::Setup(gfxSetup);
    this->clearState = ClearState::ClearAll(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), 1.0f, 0);

    // setup projection and view matrices
    const float32 fbWidth = (const float32) Gfx::DisplayAttrs().FramebufferWidth;
    const float32 fbHeight = (const float32) Gfx::DisplayAttrs().FramebufferHeight;
    this->proj = glm::perspectiveFov(glm::radians(45.0f), fbWidth, fbHeight, 0.1f, 1000.0f);
    this->view = glm::lookAt(glm::vec3(0.0f, 2.5f, 0.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // setup chunks
    for (int i = 0, x = 0; x < NumChunksX; x++) {
        for (int z = 0; z < NumChunksZ; z++, i++) {
            const glm::vec3& pos = glm::vec3(x-NumChunksX/2, 0.0f, z-NumChunksZ/2) * glm::vec3(20.0f, 1.0f, 20.0f);
            this->chunks[i].setup(gfxSetup, pos);
        }
    }
    return App::OnInit();
}

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnRunning() {
    this->frameIndex++;
    this->update_camera();
    Gfx::ApplyDefaultRenderTarget(this->clearState);
    int updateIndex = this->frameIndex;
    for (auto& chunk : this->chunks) {
        chunk.update_shader_params(this->view, this->proj);
        updateIndex = chunk.update_volume(updateIndex);
        chunk.update_mesh();
        Gfx::UpdateVertices(chunk.mesh, chunk.vertices, chunk.numQuads*6*sizeof(Chunk::Vertex));
        Gfx::ApplyDrawState(chunk.drawState);
        Gfx::ApplyUniformBlock(chunk.vsParams);
        Gfx::Draw(PrimitiveGroup(0, chunk.numQuads*6));
    }
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
VoxelTest::update_camera() {
    float32 angle = this->frameIndex * 0.01f;
    const glm::vec3 center(0.0f, 0.0f, 0.0f);
    const glm::vec3 viewerPos(sin(angle)* 100.0f, 50.0f, cos(angle) * 100.0f);
    this->view = glm::lookAt(viewerPos + center, center, glm::vec3(0.0f, 1.0f, 0.0f));
}

//------------------------------------------------------------------------------
void
Chunk::setup(const GfxSetup& gfxSetup, const glm::vec3& p) {

    this->pos = p;
    this->model = glm::translate(glm::mat4(), this->pos);
    this->vsParams.NormalTable[0] = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    this->vsParams.NormalTable[1] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    this->vsParams.NormalTable[2] = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);
    this->vsParams.NormalTable[3] = glm::vec4(0.0f, -1.0f, 0.0f, 0.0f);
    this->vsParams.NormalTable[4] = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);
    this->vsParams.NormalTable[5] = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

    // init stb_voxel_render
    stbvox_init_mesh_maker(&this->meshMaker);
    stbvox_set_default_mesh(&this->meshMaker, 0);
    stbvox_set_input_stride(&this->meshMaker, SizeY*SizeZ, SizeZ);
    stbvox_set_input_range(&this->meshMaker, 1, 1, 1, SizeX-1, SizeY-1, SizeZ-1);
    stbvox_input_description* desc = stbvox_get_input_description(&this->meshMaker);
    desc->blocktype = &(this->blocks[0][0][0]);
    desc->rgb = &(this->colors[0][0][0]);

    // dynamic vertex buffer and static index buffer
    // FIXME: only create a single index buffer
    static const int maxNumQuads = MaxNumVertices / 4;
    static const int numVertices = maxNumQuads * 4;
    static const int numIndices = maxNumQuads * 6;
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
    for (int quadIndex = 0; quadIndex < maxNumQuads; quadIndex++) {
        int baseVertexIndex = quadIndex * 4;
        int ii = quadIndex * 6;
        indices[ii]   = baseVertexIndex + 0;
        indices[ii+1] = baseVertexIndex + 1;
        indices[ii+2] = baseVertexIndex + 2;
        indices[ii+3] = baseVertexIndex + 0;
        indices[ii+4] = baseVertexIndex + 2;
        indices[ii+5] = baseVertexIndex + 3;
    }
    this->mesh = Gfx::CreateResource(meshSetup, indices, sizeof(indices));

    // shader and drawstate
    Id shd = Gfx::CreateResource(Shaders::Voxel::Setup());
    auto dss = DrawStateSetup::FromMeshAndShader(this->mesh, shd);
    dss.DepthStencilState.DepthCmpFunc = CompareFunc::LessEqual;
    dss.DepthStencilState.DepthWriteEnabled = true;
    dss.RasterizerState.CullFaceEnabled = true;
    dss.RasterizerState.SampleCount = gfxSetup.SampleCount;
    this->drawState = Gfx::CreateResource(dss);

    // fill block types once
    Memory::Clear(this->blocks, sizeof(this->blocks));
    for (int x = 1; x < SizeX-1; x++) {
        for (int y = 1; y < SizeY-1; y++) {
            for (int z = 1; z < SizeZ-1; z++) {
                this->blocks[x][y][z] = 1;
            }
        }
    }
}

//------------------------------------------------------------------------------
void
Chunk::update_shader_params(const glm::mat4& view, const glm::mat4& proj) {

    this->vsParams.ModelViewProjection = proj * view * this->model;
    this->vsParams.Model = this->model;
    this->vsParams.LightDir = glm::normalize(glm::vec3(1.0f, 0.2f, 1.0f));
    float transform[3][3];
    stbvox_get_transform(&this->meshMaker, transform);
    this->vsParams.Scale.x = transform[0][0];
    this->vsParams.Scale.y = transform[0][1];
    this->vsParams.Scale.z = transform[0][2];
    this->vsParams.Translate.x = transform[1][0];
    this->vsParams.Translate.y = transform[1][1];
    this->vsParams.Translate.z = transform[1][2];
    this->vsParams.TexTranslate.x = transform[2][0];
    this->vsParams.TexTranslate.x = transform[2][1];
    this->vsParams.TexTranslate.x = transform[2][2];
}

//------------------------------------------------------------------------------
int32
Chunk::update_volume(int32 updateIndex) {
    uint8 r = updateIndex;
    uint8 g = updateIndex + 33;
    uint8 b = updateIndex + 55;
    for (int y = 0; y < SizeY; y++) {
        for (int z = 0; z < SizeZ; z++) {
            for (int x = 0; x < SizeX; x++) {
                stbvox_rgb& c = this->colors[x][y][z];
                c.r = r; c.g = g; c.b = b;
                r += 1;
                g += 1;
                b += 1;
            }
        }
    }
    return updateIndex += 100;
}

//------------------------------------------------------------------------------
void
Chunk::update_mesh() {
    stbvox_reset_buffers(&this->meshMaker);
    stbvox_set_buffer(&this->meshMaker, 0, 0, this->vertices, sizeof(this->vertices));
    stbvox_make_mesh(&this->meshMaker);
    this->numQuads = stbvox_get_quad_count(&this->meshMaker, 0);
}
