#pragma once
//------------------------------------------------------------------------------
/** 
    @class VisTree
    @brief sparse quad-tree for LOD and visibility detection
*/
#include "Core/Types.h"
#include "Core/Containers/Array.h"
#include "glm/vec3.hpp"
#include "GeomPool.h"
#include "GeomMesher.h"
#include "VisNode.h"
#include "VisBounds.h"

class VisTree {
public:
    /// number of levels, the most detailed level is 0
    static const int NumLevels = 8;

    /// setup the vistree
    void Setup(int displayWidth, float fov);
    /// discard the vistree
    void Discard();

    /// get node by index
    VisNode& At(Oryol::int16 nodeIndex);
    /// allocate and init a node
    Oryol::int16 AllocNode();
    /// split a node (create child nodes)
    void Split(Oryol::int16 nodeIndex);
    /// merge a node, frees all child nodes recursively
    void Merge(Oryol::int16 nodeIndex);
    /// compute the screen-space error for a bounding rect and viewer pos x,y
    float ScreenSpaceError(const VisBounds& bounds, int lvl, int x, int y) const;
    /// traverse the tree, deciding which nodes to render, with viewer pos x,y
    void Traverse(int x, int y);
    /// internal, recursive traversal method
    void traverse(Oryol::int16 nodeIndex, const VisBounds& bounds, int lvl, int x, int y);

    /// get a node's bounds
    static VisBounds Bounds(int lvl, int x, int y);
    /// compute translation vector for a bounds origin
    glm::vec3 Translation(int x0, int y0);
    /// compute scale vector for a bounds rect
    glm::vec3 Scale(int x0, int x1, int y0, int y1);

    float K;
    static const int MaxNumNodes = 2048;
    VisNode nodes[MaxNumNodes];
    Oryol::Array<Oryol::int16> freeNodes;
    Oryol::int16 rootNode;
};
