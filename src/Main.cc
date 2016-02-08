//------------------------------------------------------------------------------
//  VoxelTest Main.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/App.h"
#include "Gfx/Gfx.h"
#include "Dbg/Dbg.h"
#include "Time/Clock.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/noise.hpp"
#include "shaders.h"
#include "GeomPool.h"
#include "GeomMesher.h"
#include "VoxelGenerator.h"
#include "VisMap.h"
#include "VisTree.h"

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
    int32 lastFrameIndex = -1;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 lightDir;
    ClearState clearState;

    GeomPool geomPool;
    GeomMesher geomMesher;
    VoxelGenerator voxelGenerator;
//    VisMap visMap;
    VisTree visTree;
    Array<int> displayGeoms;
};
OryolMain(VoxelTest);

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnInit() {
    auto gfxSetup = GfxSetup::WindowMSAA4(800, 600, "Oryol Voxel Test");
    gfxSetup.SetPoolSize(GfxResourceType::DrawState, 512);
    gfxSetup.SetPoolSize(GfxResourceType::Mesh, 512);
    Gfx::Setup(gfxSetup);
    this->clearState = ClearState::ClearAll(glm::vec4(0.2f, 0.2f, 0.5f, 1.0f), 1.0f, 0);
    Dbg::Setup();

    const float32 fbWidth = (const float32) Gfx::DisplayAttrs().FramebufferWidth;
    const float32 fbHeight = (const float32) Gfx::DisplayAttrs().FramebufferHeight;
    this->proj = glm::perspectiveFov(glm::radians(45.0f), fbWidth, fbHeight, 0.1f, 3000.0f);
    this->view = glm::lookAt(glm::vec3(0.0f, 2.5f, 0.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    this->lightDir = glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f));

    this->displayGeoms.Reserve(256);
    this->geomPool.Setup(gfxSetup);
    this->geomMesher.Setup();
    this->visTree.Setup(fbWidth, glm::radians(45.0f));

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

/*
    const int changeFrames = 200;
    if ((this->frameIndex/changeFrames) != this->lastFrameIndex) {
        this->lastFrameIndex = this->frameIndex/changeFrames;
*/

/*
    if (1 == this->frameIndex) {
        this->displayGeoms.Clear();
        this->geomPool.FreeAll();

//        int lvl = this->lastFrameIndex % 5;
        const int lvl = 0;
        int x0, x1, y0, y1;
//        for (int x = 0; x < (1<<lvl); x++) {
//            for (int y = 0; y < (1<<lvl); y++) {
                VisTree::Address addr(0, 0, 0);
                VisTree::Bounds bounds = this->visTree.ComputeBounds(addr);
                glm::vec3 t = this->visTree.Translation(bounds.x0, bounds.y0);
                glm::vec3 s = this->visTree.Scale(bounds.x0, bounds.x1, bounds.y0, bounds.y1);
                Volume vol = this->voxelGenerator.Gen(bounds.x0, bounds.x1, bounds.y0, bounds.y1);

                GeomMesher::Result meshResult;
                this->geomMesher.Start();
                this->geomMesher.StartVolume(vol);
                do {
                    meshResult = this->geomMesher.Meshify();
                    meshResult.Scale = s;
                    meshResult.Translate = t;
                    if (meshResult.BufferFull) {
                        this->bake_geom(meshResult);
                    }
                }
                while (!meshResult.VolumeDone);
                this->bake_geom(meshResult);
//            }
//        }
    }
*/
    const glm::mat4 mvp = this->proj * this->view;
    const int numGeoms = this->displayGeoms.Size();
    int numQuads = 0;
    for (int i = 0; i < numGeoms; i++) {
        Geom& geom = this->geomPool.GeomAt(this->displayGeoms[i]);
        geom.VSParams.ModelViewProjection = mvp;
        Gfx::ApplyDrawState(geom.DrawState);
        Gfx::ApplyUniformBlock(geom.VSParams);
        Gfx::Draw(PrimitiveGroup(0, geom.NumQuads*6));
        numQuads += geom.NumQuads;
    }
    Dbg::PrintF("draws: %d\n\rtris: %d", numGeoms, numQuads*2);
    Dbg::DrawTextBuffer();
    Gfx::CommitFrame();

    return Gfx::QuitRequested() ? AppState::Cleanup : AppState::Running;
}

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnCleanup() {
    this->visTree.Discard();
    this->geomMesher.Discard();
    this->geomPool.Discard();
    Dbg::Discard();
    Gfx::Discard();
    return App::OnCleanup();
}

//------------------------------------------------------------------------------
void
VoxelTest::update_camera() {
    float32 angle = this->frameIndex * 0.0025f;
    const glm::vec3 center(100.0, 0.0f, 100.0f);
    const glm::vec3 viewerPos(sin(angle)* 1000.0f, 200.0f, cos(angle) * 1000.0f);
    this->view = glm::lookAt(viewerPos + center, center, glm::vec3(0.0f, 1.0f, 0.0f));
}
