//------------------------------------------------------------------------------
//  VoxelTest Main.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/App.h"
#include "Gfx/Gfx.h"
#include "Input/Input.h"
#include "Dbg/Dbg.h"
#include "Time/Clock.h"
#include "shaders.h"
#include "GeomPool.h"
#include "GeomMesher.h"
#include "VoxelGenerator.h"
#include "VisTree.h"
#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"

using namespace Oryol;

const int MaxChunksGeneratedPerFrame = 1;

class VoxelTest : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();

    void init_blocks(int frameIndex);
    int bake_geom(const GeomMesher::Result& meshResult);
    void handle_input();

    int32 frameIndex = 0;
    int32 lastFrameIndex = -1;
    glm::vec3 lightDir;
    ClearState clearState;

    Camera camera;
    GeomPool geomPool;
    GeomMesher geomMesher;
    VoxelGenerator voxelGenerator;
    VisTree visTree;
};
OryolMain(VoxelTest);

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnInit() {
    auto gfxSetup = GfxSetup::WindowMSAA4(800, 600, "Oryol Voxel Test");
    gfxSetup.SetPoolSize(GfxResourceType::DrawState, 1024);
    gfxSetup.SetPoolSize(GfxResourceType::Mesh, 1024);
    Gfx::Setup(gfxSetup);
    this->clearState = ClearState::ClearAll(glm::vec4(0.2f, 0.2f, 0.5f, 1.0f), 1.0f, 0);
    Input::Setup();
    Dbg::Setup();

    const float32 fbWidth = (const float32) Gfx::DisplayAttrs().FramebufferWidth;
    const float32 fbHeight = (const float32) Gfx::DisplayAttrs().FramebufferHeight;
    this->camera.Setup(glm::vec3(4096, 128, 4096), glm::radians(45.0f), fbWidth, fbHeight, 0.1f, 10000.0f);
    this->lightDir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.25f));

    this->geomPool.Setup(gfxSetup);
    this->geomMesher.Setup();
    this->visTree.Setup(fbWidth, glm::radians(45.0f));

    return App::OnInit();
}

//------------------------------------------------------------------------------
int
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
        return geomIndex;
    }
    else {
        return VisNode::EmptyGeom;
    }
}

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnRunning() {
    this->frameIndex++;
    this->handle_input();

    Gfx::ApplyDefaultRenderTarget(this->clearState);

    // traverse the vis-tree
    this->visTree.Traverse(this->camera);
    // free any geoms to be freed
    while (!this->visTree.freeGeoms.Empty()) {
        int geom = this->visTree.freeGeoms.PopBack();
        this->geomPool.Free(geom);
    }
    // init new geoms
    if (!this->visTree.geomGenJobs.Empty()) {
        int numProcessedJobs = 0;
        while ((numProcessedJobs < MaxChunksGeneratedPerFrame) && !this->visTree.geomGenJobs.Empty()) {
            numProcessedJobs++;
            int16 geoms[VisNode::NumGeoms];
            int numGeoms = 0;
            VisTree::GeomGenJob job = this->visTree.geomGenJobs.PopBack();
            Volume vol = this->voxelGenerator.GenSimplex(job.Bounds);
//Volume vol = this->voxelGenerator.GenDebug(job.Bounds, job.Level);
            GeomMesher::Result meshResult;
            this->geomMesher.Start();
            this->geomMesher.StartVolume(vol);
            do {
                meshResult = this->geomMesher.Meshify();
                meshResult.Scale = job.Scale;
                meshResult.Translate = job.Translate;
                if (meshResult.BufferFull) {
                    int geom = this->bake_geom(meshResult);
                    o_assert(numGeoms < VisNode::NumGeoms);
                    geoms[numGeoms++] = geom;
                }
            }
            while (!meshResult.VolumeDone);
            int geom = this->bake_geom(meshResult);
            o_assert(numGeoms < VisNode::NumGeoms);
            geoms[numGeoms++] = geom;
            this->visTree.ApplyGeoms(job.NodeIndex, geoms, numGeoms);
        }
    }

    const int numDrawNodes = this->visTree.drawNodes.Size();
    int numQuads = 0;
    int numGeoms = 0;
    for (int i = 0; i < numDrawNodes; i++) {
        const VisNode& node = this->visTree.NodeAt(this->visTree.drawNodes[i]);
        for (int geomIndex = 0; geomIndex < VisNode::NumGeoms; geomIndex++) {
            if (node.geoms[geomIndex] >= 0) {
                Geom& geom = this->geomPool.GeomAt(node.geoms[geomIndex]);
                geom.VSParams.ModelViewProjection = this->camera.ViewProj;
                Gfx::ApplyDrawState(geom.DrawState);
                Gfx::ApplyUniformBlock(geom.VSParams);
                Gfx::Draw(PrimitiveGroup(0, geom.NumQuads*6));
                numQuads += geom.NumQuads;
                numGeoms++;
            }
        }
    }
    Dbg::PrintF("\n\r"
                " LMB+Mouse or AWSD to move, RMB+Mouse to look around\n\n\r"
                " draws: %d\n\r"
                " tris: %d\n\r"
                " avail geoms: %d\n\r"
                " avail nodes: %d\n\r"
                " pending chunks: %d\n\r",
                numGeoms, numQuads*2,
                this->geomPool.freeGeoms.Size(),
                this->visTree.freeNodes.Size(),
                this->visTree.geomGenJobs.Size());
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
    Input::Discard();
    Gfx::Discard();
    return App::OnCleanup();
}

//------------------------------------------------------------------------------
void
VoxelTest::handle_input() {
    glm::vec3 move;
    glm::vec2 rot;
    const float vel = 0.75f;
    const Keyboard& kbd = Input::Keyboard();
    if (kbd.Attached) {
        if (kbd.KeyPressed(Key::W) || kbd.KeyPressed(Key::Up)) {
            move.z -= vel;
        }
        if (kbd.KeyPressed(Key::S) || kbd.KeyPressed(Key::Down)) {
            move.z += vel;
        }
        if (kbd.KeyPressed(Key::A) || kbd.KeyPressed(Key::Left)) {
            move.x -= vel;
        }
        if (kbd.KeyPressed(Key::D) || kbd.KeyPressed(Key::Right)) {
            move.x += vel;
        }
    }
    const Mouse& mouse = Input::Mouse();
    if (mouse.Attached) {
        if (mouse.ButtonPressed(Mouse::Button::LMB)) {
            move.z -= vel;
        }
        if (mouse.ButtonPressed(Mouse::Button::LMB) || mouse.ButtonPressed(Mouse::Button::RMB)) {
            rot = mouse.Movement * glm::vec2(-0.01, -0.007f);
        }
    }
    this->camera.MoveRotate(move, rot);
}
