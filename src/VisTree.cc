//------------------------------------------------------------------------------
//  VisTree.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Config.h"
#include "VisTree.h"
#include "glm/trigonometric.hpp"

using namespace Oryol;

//------------------------------------------------------------------------------
void
VisTree::Setup(int displayWidth, float fov) {

    // compute K for screen space error computation
    // (see: http://tulrich.com/geekstuff/sig-notes.pdf )
    this->K = float(displayWidth) / (2.0f * glm::tan(fov*0.5f));

    this->drawNodes.Reserve(MaxNumNodes);
    this->freeNodes.Reserve(MaxNumNodes);
    this->geomGenJobs.Reserve(MaxNumNodes);
    this->freeGeoms.Reserve(MaxNumNodes);
    for (int i = MaxNumNodes-1; i >=0; i--) {
        this->freeNodes.Add(i);
    }
    this->rootNode = this->AllocNode();
}

//------------------------------------------------------------------------------
void
VisTree::Discard() {
    this->freeNodes.Clear();
}

//------------------------------------------------------------------------------
VisNode&
VisTree::NodeAt(int16 nodeIndex) {
    o_assert_dbg((nodeIndex >= 0) && (nodeIndex < MaxNumNodes));
    return this->nodes[nodeIndex];
}

//------------------------------------------------------------------------------
int16
VisTree::AllocNode() {
    int16 index = this->freeNodes.PopBack();
    VisNode& node = this->nodes[index];
    node.Reset();
    return index;
}

//------------------------------------------------------------------------------
void
VisTree::FreeGeoms(int16 nodeIndex) {
    VisNode& node = this->NodeAt(nodeIndex);
    for (int geomIndex = 0; geomIndex < VisNode::NumGeoms; geomIndex++) {
        if (node.geoms[geomIndex] >= 0) {
            this->freeGeoms.Add(node.geoms[geomIndex]);
            node.geoms[geomIndex] = VisNode::InvalidGeom;
        }
    }
}

//------------------------------------------------------------------------------
void
VisTree::Split(int16 nodeIndex) {
    // turns a leaf node into an inner node, frees any draw geoms and
    // and adds 4 child nodes
    VisNode& node = this->NodeAt(nodeIndex);
    o_assert_dbg(node.IsLeaf());
    this->FreeGeoms(nodeIndex);
    for (int childIndex = 0; childIndex < VisNode::NumChilds; childIndex++) {
        o_assert_dbg(VisNode::InvalidChild == node.childs[childIndex]);
        node.childs[childIndex] = this->AllocNode();
    }
    node.flags &= ~VisNode::GeomPending;
}

//------------------------------------------------------------------------------
void
VisTree::Merge(int16 nodeIndex) {
    // turns an inner node into a leaf node by recursively removing
    // children and any encountered draw geoms
    VisNode& node = this->NodeAt(nodeIndex);
    for (int childIndex = 0; childIndex < VisNode::NumChilds; childIndex++) {
        if (VisNode::InvalidChild != node.childs[childIndex]) {
            VisNode& childNode = this->NodeAt(node.childs[childIndex]);
            this->FreeGeoms(node.childs[childIndex]);
            this->Merge(node.childs[childIndex]);
            this->freeNodes.Add(node.childs[childIndex]);
            node.childs[childIndex] = VisNode::InvalidChild;
        }
    }
}

//------------------------------------------------------------------------------
float
VisTree::ScreenSpaceError(const VisBounds& bounds, int lvl, int posX, int posY) const {
    // see http://tulrich.com/geekstuff/sig-notes.pdf

    // we just fudge the geometric error of the chunk by doubling it for
    // each tree level
    const float delta = float(1<<lvl);
    const float D = float(bounds.MinDist(posX, posY)+1);
    float rho = (delta/D) * this->K;
    return rho;
}

//------------------------------------------------------------------------------
void
VisTree::Traverse(const Camera& camera) {
    // traverse the entire tree to find draw nodes
    // split and merge nodes based required LOD,
    int lvl = NumLevels;
    int nodeIndex = this->rootNode;
    int posX = camera.Pos.x;
    int posY = camera.Pos.z;
    VisBounds bounds = VisTree::Bounds(lvl, 0, 0);
    this->drawNodes.Clear();
    this->traverse(camera, nodeIndex, bounds, lvl, posX, posY);
}

//------------------------------------------------------------------------------
void
VisTree::traverse(const Camera& camera, int16 nodeIndex, const VisBounds& bounds, int lvl, int posX, int posY) {
    VisNode& node = this->NodeAt(nodeIndex);
    float rho = this->ScreenSpaceError(bounds, lvl, posX, posY);
    const float tau = 15.0f;
    if ((rho <= tau) || (0 == lvl)) {
        if (!node.IsLeaf()) {
            this->Merge(nodeIndex);
        }
        this->gatherDrawNode(camera, nodeIndex, lvl, bounds);
    }
    else {
        if (node.IsLeaf()) {
            this->Split(nodeIndex);
        }
        const int halfX = (bounds.x1 - bounds.x0)/2;
        const int halfY = (bounds.y1 - bounds.y0)/2;
        for (int x = 0; x < 2; x++) {
            for (int y = 0; y < 2; y++) {
                VisBounds childBounds;
                childBounds.x0 = bounds.x0 + x*halfX;
                childBounds.x1 = childBounds.x0 + halfX;
                childBounds.y0 = bounds.y0 + y*halfY;
                childBounds.y1 = childBounds.y0 + halfY;
                const int childIndex = (y<<1)|x;
                this->traverse(camera, node.childs[childIndex], childBounds, lvl-1, posX, posY);
            }
        }
    }
}

//------------------------------------------------------------------------------
void
VisTree::gatherDrawNode(const Camera& camera, int16 nodeIndex, int lvl, const VisBounds& bounds) {
    VisNode& node = this->NodeAt(nodeIndex);
    if (node.HasEmptyGeom()) {
        return;
    }
    if (!camera.BoxVisible(bounds.x0, bounds.x1, 0, Config::ChunkSizeXY, bounds.y0, bounds.y1)) {
        return;
    }
    this->drawNodes.Add(nodeIndex);
    if (node.NeedsGeom()) {
        // enqueue a new geom-generation job
        node.flags |= VisNode::GeomPending;
        glm::vec3 scale = Scale(bounds.x0, bounds.x1, bounds.y0, bounds.y1);
        glm::vec3 trans = Translation(bounds.x0, bounds.y0);
        this->geomGenJobs.Add(GeomGenJob(nodeIndex, lvl, bounds, scale, trans));
    }
}

//------------------------------------------------------------------------------
void
VisTree::ApplyGeoms(int16 nodeIndex, int16* geoms, int numGeoms) {
    VisNode& node = this->NodeAt(nodeIndex);
    if (node.WaitsForGeom()) {
        for (int i = 0; i < VisNode::NumGeoms; i++) {
            o_assert_dbg(VisNode::InvalidGeom == node.geoms[i]);
            if (i < numGeoms) {
                node.geoms[i] = geoms[i];
            }
            else {
                node.geoms[i] = VisNode::InvalidGeom;
            }
        }
        node.flags &= ~VisNode::GeomPending;
    }
    else {
        // if the node didn't actually wait for geoms any longer,
        // immediately kill the geoms
        for (int i = 0; i < numGeoms; i++) {
            o_assert_dbg(VisNode::InvalidGeom != geoms[i]);
            this->freeGeoms.Add(geoms[i]);
        }
    }
}

//------------------------------------------------------------------------------
VisBounds
VisTree::Bounds(int lvl, int x, int y) {
    o_assert_dbg(lvl <= NumLevels);
    // level 0 is most detailed, level == NumLevels is the root node
    int dim = (1<<lvl) * Config::ChunkSizeXY;
    VisBounds bounds;
    bounds.x0 = (x>>lvl) * dim;
    bounds.x1 = bounds.x0 + dim;
    bounds.y0 = (y>>lvl) * dim;
    bounds.y1 = bounds.y0 + dim;
    return bounds;
}

//------------------------------------------------------------------------------
glm::vec3
VisTree::Translation(int x0, int y0) {
    return glm::vec3(x0, y0, 0.0f);
}

//------------------------------------------------------------------------------
glm::vec3
VisTree::Scale(int x0, int x1, int y0, int y1) {
    const float s = Config::ChunkSizeXY;
    return glm::vec3(float(x1-x0)/s, float(y1-y0)/s, 1.0f);
}
