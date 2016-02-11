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
#include "VisTree.h"

using namespace Oryol;

const int MaxChunksGeneratedPerFrame = 2;

class VoxelTest : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();

    void update_camera();
    void init_blocks(int frameIndex);
    int bake_geom(const GeomMesher::Result& meshResult);

    int32 frameIndex = 0;
    int32 lastFrameIndex = -1;
    glm::mat4 view;
    glm::mat4 proj;
    glm::vec3 lightDir;
    ClearState clearState;

    GeomPool geomPool;
    GeomMesher geomMesher;
    VoxelGenerator voxelGenerator;
    VisTree visTree;
    int viewerX = 4096;
    int viewerY = 4096;
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
    this->proj = glm::perspectiveFov(glm::radians(45.0f), fbWidth, fbHeight, 0.1f, 10000.0f);
    this->view = glm::lookAt(glm::vec3(0.0f, 2.5f, 0.0f), glm::vec3(0.0f, 0.0f, -10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
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
    return InvalidIndex;
}

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnRunning() {
    this->frameIndex++;
    this->viewerX = (this->viewerX + 1) % 8192;

    this->update_camera();

    Gfx::ApplyDefaultRenderTarget(this->clearState);

    // traverse the vis-tree
    this->visTree.Traverse(this->viewerX, this->viewerY);
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
                    if ((InvalidIndex != geom) && (numGeoms < VisNode::NumGeoms)) {
                        geoms[numGeoms++] = geom;
                    }
                }
            }
            while (!meshResult.VolumeDone);
            int geom = this->bake_geom(meshResult);
            if ((InvalidIndex != geom) && (numGeoms < VisNode::NumGeoms)) {
                geoms[numGeoms++] = geom;
            }
            this->visTree.ApplyGeoms(job.NodeIndex, geoms, numGeoms);
        }
    }

    const glm::mat4 mvp = this->proj * this->view;
    const int numDrawNodes = this->visTree.drawNodes.Size();
    int numQuads = 0;
    int numGeoms = 0;
    for (int i = 0; i < numDrawNodes; i++) {
        const VisNode& node = this->visTree.NodeAt(this->visTree.drawNodes[i]);
        for (int geomIndex = 0; geomIndex < VisNode::NumGeoms; geomIndex++) {
            if (node.geoms[geomIndex] != InvalidIndex) {
                Geom& geom = this->geomPool.GeomAt(node.geoms[geomIndex]);
                geom.VSParams.ModelViewProjection = mvp;
                Gfx::ApplyDrawState(geom.DrawState);
                Gfx::ApplyUniformBlock(geom.VSParams);
                Gfx::Draw(PrimitiveGroup(0, geom.NumQuads*6));
                numQuads += geom.NumQuads;
                numGeoms++;
            }
        }
    }
    Dbg::PrintF("draws: %d\n\r"
                "tris: %d\n\r"
                "avail geoms: %d\n\r"
                "avail nodes: %d\n\r",
                numGeoms, numQuads*2,
                this->geomPool.freeGeoms.Size(),
                this->visTree.freeNodes.Size());
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
    const glm::vec3 center(this->viewerX, 32.0f, this->viewerY);
    const glm::vec3 viewerPos(sin(angle)* 200.0f, 48.0f, cos(angle) * 200.0f);
    this->view = glm::lookAt(viewerPos + center, center, glm::vec3(0.0f, 1.0f, 0.0f));
}
