//------------------------------------------------------------------------------
//  VisTree.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "Config.h"
#include "VisTree.h"

using namespace Oryol;

//------------------------------------------------------------------------------
void
VisTree::Setup() {
    this->freeNodes.Reserve(MaxNumNodes);
    for (int i = MaxNumNodes-1; i >=0; i--) {
        this->freeNodes.Add(i);
    }
    this->rootNode = InvalidIndex;
}

//------------------------------------------------------------------------------
void
VisTree::Discard() {
    this->freeNodes.Clear();
}

//------------------------------------------------------------------------------
int16
VisTree::AllocNode() {
    int16 index = this->freeNodes.PopBack();
    Node& node = this->nodes[index];
    node.flags = Node::Flags::None;
    for (int i = 0; i < 4; i++) {
        node.child[i] = InvalidIndex;
    }
    return index;
}

//------------------------------------------------------------------------------
void
VisTree::FreeGeoms(GeomPool& geomPool, int16 nodeIndex) {
    Node& curNode = this->nodes[nodeIndex];
    o_assert_dbg(curNode.flags & Node::Leaf);
    for (int i=0; i<4; i++) {
        if (curNode.geom[i] != InvalidIndex) {
            geomPool.Free(curNode.geom[i]);
            curNode.geom[i] = InvalidIndex;
        }
    }
}

//------------------------------------------------------------------------------
void
VisTree::FreeNodes(GeomPool& geomPool, int16 index) {
    o_assert_dbg(InvalidIndex != index);
    Node& curNode = this->nodes[index];
    if (curNode.IsLeaf()) {
        this->FreeGeoms(geomPool, index);
    }
    else {
        for (int i = 0; i < 4; i++) {
            if (InvalidIndex != curNode.child[i]) {
                this->FreeNodes(geomPool, curNode.child[i]);
                curNode.child[i] = InvalidIndex;
            }
        }
    }
    this->freeNodes.Add(index);
}

//------------------------------------------------------------------------------
void
VisTree::UpdateLod(GeomPool& geomPool, GeomMesher& geomMesher, uint8 x, uint8 y) {
    Address addr(0, x, y);
    Bounds bounds(0, (1<<NumLevels), 0, (1<<NumLevels));
    this->updateLod(geomPool, geomMesher, this->rootNode, bounds, addr);
}

//------------------------------------------------------------------------------
void
VisTree::updateLod(GeomPool& geomPool, GeomMesher& geomMesher, int16 nodeIndex, const Bounds& bounds, const Address& addr) {

    // if execution reached here, this node cannot be a leaf node
    Node& curNode = this->nodes[nodeIndex];
    if (curNode.IsLeaf()) {
        this->FreeGeoms(geomPool, nodeIndex);
        curNode.flags &= ~Node::Leaf;
    }

    // FIXME: for each quad, if addr is inside quad, recurse,
    // otherwise compute distance between quad bounds and 'addr' and
    // compute an LOD value
    int halfX = (bounds.x1 - bounds.x0)/2;
    int halfY = (bounds.y1 - bounds.y0)/2;
    for (int x=0; x<2; x++) {
        for (int y=0; y<2; y++) {

            // if not happened yet, allocate child nodes
            const int childIndex = (y<<1) | x;
            const int childLevel = addr.level + 1;
            if (InvalidIndex == curNode.child[childIndex]) {
                curNode.child[childIndex] = this->AllocNode();
            }

            Bounds childBounds;
            childBounds.x0 = bounds.x0 + x*halfX;
            childBounds.x1 = bounds.x1 + halfX;
            childBounds.y0 = bounds.y0 + y*halfY;
            childBounds.y1 = bounds.y1 + halfY;
            if (childBounds.Inside(addr.x, addr.y)) {
                if (childLevel < NumLevels) {
                    // recurse
                    Address childAddr;
                    childAddr.level = childLevel;
                    childAddr.x = addr.x;
                    childAddr.y = addr.y;
                    this->updateLod(geomPool, geomMesher, curNode.child[childIndex], childBounds, childAddr);
                }
            }
            else {
                // setup direct child as leaf node
                if (!this->nodes[curNode.child[childIndex]].IsLeaf()) {
                    this->CreateLeaf(geomPool, curNode.child[childIndex], childBounds);
                    o_assert_dbg(this->nodes[curNode.child[childIndex]].IsLeaf());
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void
VisTree::CreateLeaf(GeomPool& geomPool, int16 nodeIndex, const Bounds& bounds) {
    o_assert_dbg(!this->nodes[nodeIndex].IsLeaf());

    Node& curNode = this->nodes[nodeIndex];

    // recursively free any child nodes
    for (int i = 0; i < 4; i++) {
        if (InvalidIndex != curNode.child[i]) {
            this->FreeNodes(geomPool, curNode.child[i]);
            curNode.child[i] = InvalidIndex;
        }
    }

    FIXME FIXME FIXME
}

//------------------------------------------------------------------------------
VisTree::Bounds
VisTree::ComputeBounds(const Address& addr) {
    o_assert_dbg(addr.level < NumLevels);
    int shift = NumLevels - addr.level - 1;
    int dim = (1<<shift) * Config::ChunkSizeXY;
    Bounds bounds;
    bounds.x0 = (addr.x>>shift) * dim;
    bounds.x1 = bounds.x0 + dim;
    bounds.y0 = (addr.y>>shift) * dim;
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
