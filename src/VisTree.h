#pragma once
//------------------------------------------------------------------------------
/** 
    @class VisTree
    @brief sparse quad-tree for LOD and visibility
    
    A node address consists of 3 bytes, a level (0..7), an x and a 
    y coordinate.

    uint8 level
    uint8 x
    uint8 y
    
    The x/y coordinates have as many valid bits as 'level+1', starting
    at the most significant bit. The 2 coordinate bits for X/Y define
    the quadrant. The X coordinate bit selects left (0) and right (1)
    half, the Y coordinate bit selects upper (0) and lower (1) half.
    
    A node can be an inner node (has 4 children), or a leaf node (has
    at least 1 geom for rendering). 
    
    Thus to find the smallest existing node for a given coordinate
    (lvl,x,y):
    
    int nodeIndex = this->rootNode;
    for (int l=0; l<lvl; l++) {
        int childIndex = ((x<<lvl)&128) ? 1:0;
        childIndex |= ((y<<lvl)&128) ? 2:0;
        nodeIndex = this->nodes[nodeIndex].child[childIndex];
    }
    nodeIndex now points to the addressed node
*/
#include "Core/Types.h"
#include "Core/Containers/Array.h"
#include "glm/vec3.hpp"
#include "GeomPool.h"
#include "GeomMesher.h"

class VisTree {
public:
    /// number of levels
    static const int NumLevels = 8;
    /// a node in the sparse quad tree
    class Node {
    public:
        enum Flags : Oryol::int32 {
            None = 0,
            Leaf = (1<<0),          // set if this is a leaf node
            CullInside = (1<<2),    // set if fully inside view volume
            CullOutside = (1<<3),   // set if fully outside view volume
        };
        Oryol::uint8 flags;
        union {
            Oryol::int16 child[4];  // child nodes (if inner node)
            Oryol::int16 geom[4];  // up to 4 geoms (if leaf node)
        };
        /// is the node a leaf node?
        bool IsLeaf() const {
            return 0 != (this->flags & Flags::Leaf);
        }
    };

    /// an address of a node
    class Address {
    public:
        Address() : level(0), x(0), y(0) { };
        Address(Oryol::uint8 lvl, Oryol::uint8 x_, Oryol::uint8 y_) : level(lvl), x(x_), y(y_) { };

        Oryol::uint8 level;
        Oryol::uint8 x;
        Oryol::uint8 y;
    };

    /// a bounding rectangle
    class Bounds {
    public:
        Bounds() : x0(0), x1(0), y0(0), y1(0) { };
        Bounds(int x0_, int x1_, int y0_, int y1_) : x0(x0_), x1(x1_), y0(y0_), y1(y1_) { };
        bool Inside(int x, int y) const {
            return (x>=this->x0) && (x<this->x1) && (y>=this->y0) && (y<this->y1);
        };
        int x0, x1, y0, y1;
    };

    /// setup the vistree
    void Setup();
    /// discard the vistree
    void Discard();

    /// allocate a new node, return node index
    Oryol::int16 AllocNode();
    /// free any geoms in a node
    void FreeGeoms(GeomPool& geomPool, Oryol::int16 nodeIndex);
    /// recursively free node and child nodes
    void FreeNodes(GeomPool& geomPool, Oryol::int16 nodeIndex);
    /// update node LOD state by viewer pos
    void UpdateLod(GeomPool& geomPool, GeomMesher& geomMesher, Oryol::uint8 x, Oryol::uint8 y);
    /// internal, recursive updateLod() method
    void updateLod(GeomPool& geomPool, GeomMesher& geomMesher, Oryol::int16 nodeIndex, const Bounds& bounds, const Address& addr);
    /// turn a node into a leaf node
    void CreateLeaf(GeomPool& geomPool, Oryol::int16 nodeIndex, const Bounds& bounds);

    /// get bounding rect of a node defined by address
    static Bounds ComputeBounds(const Address& addr);
    /// compute translation vector for a bounds origin
    glm::vec3 Translation(int x0, int y0);
    /// compute scale vector for a bounds rect
    glm::vec3 Scale(int x0, int x1, int y0, int y1);

    /// node pool
    static const int MaxNumNodes = 2048;
    Node nodes[MaxNumNodes];

    /// currently free nodes
    Oryol::Array<Oryol::int16> freeNodes;
    /// root node index
    Oryol::int16 rootNode;
};
