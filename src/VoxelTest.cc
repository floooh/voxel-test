//------------------------------------------------------------------------------
//  VoxelTest.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Core/App.h"
#include "Gfx/Gfx.h"
#include "glm/vec4.hpp"

using namespace Oryol;

class VoxelTest : public App {
public:
    AppState::Code OnInit();
    AppState::Code OnRunning();
    AppState::Code OnCleanup();

    ClearState clearState;
};
OryolMain(VoxelTest);

//------------------------------------------------------------------------------
AppState::Code
VoxelTest::OnInit() {
    Gfx::Setup(GfxSetup::Window(800, 600, "Oryol Voxel Test"));
    this->clearState = ClearState::ClearAll(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), 1.0f, 0);
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

