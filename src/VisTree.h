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
#include "Camera.h"

class VisTree {
public:
    /// number of levels, the most detailed level is 0
    static const int NumLevels = 8;

    /// setup the vistree
    void Setup(int displayWidth, float fov);
    /// discard the vistree
    void Discard();

    /// get node by index
    VisNode& NodeAt(Oryol::int16 nodeIndex);
    /// allocate and init a node
    Oryol::int16 AllocNode();
    /// free any geoms in a node (non-recursive)
    void FreeGeoms(Oryol::int16 nodeIndex);
    /// split a node (create child nodes)
    void Split(Oryol::int16 nodeIndex);
    /// merge a node, frees all child nodes recursively
    void Merge(Oryol::int16 nodeIndex);
    /// compute the screen-space error for a bounding rect and viewer pos x,y
    float ScreenSpaceError(const VisBounds& bounds, int lvl, int x, int y) const;
    /// traverse the tree, deciding which nodes to render
    void Traverse(const Camera& camera);
    /// apply geoms to a node
    void ApplyGeoms(Oryol::int16 nodeIndex, Oryol::int16* geoms, int numGeoms);
    /// internal, recursive traversal method
    void traverse(const Camera& camera, Oryol::int16 nodeIndex, const VisBounds& bounds, int lvl, int x, int y);
    /// gather a drawable node, prepare for drawing if needed
    void gatherDrawNode(const Camera& camera, Oryol::int16 nodeIndex, int lvl, const VisBounds& bounds);
    /// invalidate any child nodes (free geoms, free nodes)
    void invalidateChildNodes(Oryol::int16 nodeIndex);

    /// get a node's bounds
    static VisBounds Bounds(int lvl, int x, int y);
    /// compute translation vector for a bounds
    static glm::vec3 Translation(const VisBounds& bounds);
    /// compute scale vector for a bounds rect
    static glm::vec3 Scale(const VisBounds& bounds);

    struct GeomGenJob {
        GeomGenJob() : NodeIndex(Oryol::InvalidIndex), Level(0) { }
        GeomGenJob(Oryol::int16 nodeIndex, int lvl, const VisBounds& bounds, const glm::vec3& scale, const glm::vec3& trans) :
            NodeIndex(nodeIndex), Level(lvl), Bounds(bounds), Scale(scale), Translate(trans) { }

        Oryol::int16 NodeIndex;
        int Level;
        VisBounds Bounds;
        glm::vec3 Scale;
        glm::vec3 Translate;
    };

    float K;
    static const int MaxNumNodes = 1024;
    VisNode nodes[MaxNumNodes];
    Oryol::Array<Oryol::int16> freeNodes;
    Oryol::Array<Oryol::int16> drawNodes;
    Oryol::Array<GeomGenJob> geomGenJobs;
    Oryol::Array<Oryol::int16> freeGeoms;
    Oryol::Array<Oryol::int16> traverseStack;
    Oryol::int16 rootNode;
};
