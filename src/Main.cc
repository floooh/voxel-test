//------------------------------------------------------------------------------
//  VoxelTest Main.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/App.h"
#include "Gfx/Gfx.h"
#include "Time/Clock.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "shaders.h"
#include "GeomPool.h"
#include "GeomMesher.h"

using namespace Oryol;

class VoxelTest : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();

    void update_camera();
    void init_blocks(int frameIndex);
    void bake_geom(const GeomMesher::Result& meshResult);

    int32 frameIndex = 0;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 lightDir;
    ClearState clearState;

    GeomPool geomPool;
    GeomMesher geomMesher;
    Array<int> displayGeoms;

    static const int WorldSizeX = 256;
    static const int WorldSizeY = 256;
    static const int WorldSizeZ = 32;
    static const int VolumeSizeX = 64;
    static const int VolumeSizeY = 64;
    static const int VolumeSizeZ = WorldSizeZ;
    static const int NumBlockTypes = 32;

    uint8 blocks[WorldSizeX][WorldSizeY][WorldSizeZ];
};
OryolMain(VoxelTest);

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnInit() {
    auto gfxSetup = GfxSetup::WindowMSAA4(800, 600, "Oryol Voxel Test");
    Gfx::Setup(gfxSetup);
    this->clearState = ClearState::ClearAll(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), 1.0f, 0);

    const float32 fbWidth = (const float32) Gfx::DisplayAttrs().FramebufferWidth;
    const float32 fbHeight = (const float32) Gfx::DisplayAttrs().FramebufferHeight;
    this->proj = glm::perspectiveFov(glm::radians(45.0f), fbWidth, fbHeight, 0.1f, 1000.0f);
    this->view = glm::lookAt(glm::vec3(0.0f, 2.5f, 0.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    this->lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

    this->displayGeoms.Reserve(256);
    this->geomPool.Setup(gfxSetup);
    this->geomMesher.Setup();
    return App::OnInit();
}

//------------------------------------------------------------------------------
void
VoxelTest::bake_geom(const GeomMesher::Result& meshResult) {
    if (meshResult.NumQuads > 0) {
        int geomIndex = this->geomPool.Alloc();
        auto& geom = this->geomPool.GeomAt(geomIndex);
        Gfx::UpdateVertices(geom.Mesh, meshResult.Vertices, meshResult.NumBytes);
        geom.NumQuads = meshResult.NumQuads;
        geom.VSParams.Model = glm::mat4();
        geom.VSParams.LightDir = this->lightDir;
        geom.VSParams.Scale = meshResult.Scale;
        geom.VSParams.Translate = meshResult.Translate;
        geom.VSParams.TexTranslate = meshResult.TexTranslate;
        this->displayGeoms.Add(geomIndex);
    }
}

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnRunning() {
    this->frameIndex++;
    this->update_camera();

    Gfx::ApplyDefaultRenderTarget(this->clearState);

    if (1 == this->frameIndex) {
        this->displayGeoms.Clear();
        this->geomPool.FreeAll();
        Volume vol;
        vol.Blocks = &(this->blocks[0][0][0]);
        vol.ArraySize.x = WorldSizeX;
        vol.ArraySize.y = WorldSizeY;
        vol.ArraySize.z = WorldSizeZ;
        vol.Size.x = VolumeSizeX;
        vol.Size.y = VolumeSizeY;
        vol.Size.z = VolumeSizeZ;
        const int numChunksX = WorldSizeX / VolumeSizeX;
        const int numChunksY = WorldSizeY / VolumeSizeY;
        const int numChunksZ = WorldSizeZ / VolumeSizeZ;

        GeomMesher::Result meshResult;
        this->init_blocks(this->frameIndex);
        this->geomMesher.Start();
        for (int x = 0; x < numChunksX; x++) {
            vol.Offset.x = x * VolumeSizeX;
            for (int y = 0; y < numChunksY; y++) {
                vol.Offset.y = y * VolumeSizeY;
                for (int z = 0; z < numChunksZ; z++) {
                    vol.Offset.z = z * VolumeSizeZ;
                    this->geomMesher.StartVolume(vol);
                    do {
                        meshResult = this->geomMesher.Meshify();
                        if (meshResult.BufferFull) {
                            this->bake_geom(meshResult);
                        }
                    }
                    while (!meshResult.VolumeDone);
                }
            }
        }
        this->bake_geom(meshResult);
    }

    const glm::mat4 mvp = this->proj * this->view;
    const int numGeoms = this->displayGeoms.Size();
    for (int i = 0; i < numGeoms; i++) {
        Geom& geom = this->geomPool.GeomAt(this->displayGeoms[i]);
        geom.VSParams.ModelViewProjection = mvp;
        Gfx::ApplyDrawState(geom.DrawState);
        Gfx::ApplyUniformBlock(geom.VSParams);
        Gfx::Draw(PrimitiveGroup(0, geom.NumQuads*6));
    }
    Gfx::CommitFrame();

    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnCleanup() {
    this->geomMesher.Discard();
    this->geomPool.Discard();
    Gfx::Discard();
    return App::OnCleanup();
}

//------------------------------------------------------------------------------
void
VoxelTest::init_blocks(int index) {
    Memory::Clear(this->blocks, sizeof(this->blocks));

    const float dx = 1.0f / float(WorldSizeX);
    const float dy = 1.0f / float(WorldSizeY);
    glm::vec2 p(dx, dy);
    for (int x = 1; x < WorldSizeX-1; x++, p.x+=dx) {
        p.y = dy;
        for (int y = 1; y < WorldSizeY-1; y++, p.y+=dy) {
            float n = (glm::simplex(p) + 1.0f) * 0.5f;
            n += glm::simplex(p * 10.0f) * 0.3;
            n += glm::simplex(p * 40.0f) * 0.1;
            n = glm::clamp(n, 0.0f, 1.0f);
            const int max_z = 1 + (WorldSizeZ-2)*n;
            for (int z = 1; z < max_z; z++) {
                this->blocks[x][y][z] = z;
            }
        }
    }

/*
    for (int x = 1; x < WorldSizeX-1; x++) {
        for (int y = 1; y < WorldSizeY-1; y++) {
            for (int z = 1; z < WorldSizeZ-1; z++) {
                if ((z <= ((x+index)&7)) && (z <= (y&7))) {
                    uint8 blockType = (z+y+1) & (NumBlockTypes-1);;
                    this->blocks[x][y][z] = blockType;
                }
            }
        }
    }
*/
}

//------------------------------------------------------------------------------
void
VoxelTest::update_camera() {
    float32 angle = this->frameIndex * 0.005f;
    const glm::vec3 center(128.0f, 0.0f, 128.0f);
    const glm::vec3 viewerPos(sin(angle)* 200.0f, 75.0f, cos(angle) * 200.0f);
    this->view = glm::lookAt(viewerPos + center, center, glm::vec3(0.0f, 1.0f, 0.0f));
}
