//------------------------------------------------------------------------------
//  VisMap.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "VisMap.h"
#include "Core/Assertion.h"

using namespace Oryol;

//------------------------------------------------------------------------------
void
VisMap::Setup() {
    // init mipmap level start indices
    int x = MaxNumChunks;
    int offset = 0;
    for (int i = 0; i < Config::NumLevels; i++) {
        this->LevelIndex[i] = offset;
        offset += (1<<i)*(1<<i);
    }
}

//------------------------------------------------------------------------------
int
VisMap::IndexOf(int lvl, int x, int y) const {
    o_assert_dbg((lvl >= 0) && (lvl < Config::NumLevels));
    const int dim = (1<<lvl);
    o_assert_dbg((x < dim) && (y < dim));
    const int index = this->LevelIndex[lvl] + x*dim + y;
    o_assert_dbg(index < MaxNumChunks);
    return index;
}

//------------------------------------------------------------------------------
VisMap::Chunk&
VisMap::At(int lvl, int x, int y) {
    return this->Chunks[this->IndexOf(lvl, x, y)];
}

//------------------------------------------------------------------------------
const VisMap::Chunk&
VisMap::At(int lvl, int x, int y) const {
    return this->Chunks[this->IndexOf(lvl, x, y)];
}

//------------------------------------------------------------------------------
void
VisMap::Bounds(int lvl, int x, int y, int& outX0, int& outX1, int& outY0, int& outY1) const {
    o_assert((lvl >= 0) && (lvl < Config::NumLevels));
    o_assert_dbg((x < (1<<lvl)) && (y < (1<<lvl)));

    // width of one item in voxels
    const int dim = (1<<(Config::NumLevels-lvl-1)) * Config::ChunkSizeXY;
    outX0 = x*dim;
    outX1 = outX0 + dim;
    outY0 = y*dim;
    outY1 = outY0 + dim;
}

//------------------------------------------------------------------------------
glm::vec3
VisMap::Translation(int x0, int y0) {
    return glm::vec3(x0, y0, 0.0f);
}

//------------------------------------------------------------------------------
glm::vec3
VisMap::Scale(int x0, int x1, int y0, int y1) {
    const float s = Config::ChunkSizeXY;
    return glm::vec3(float(x1-x0)/s, float(y1-y0)/s, 1.0f);
}
