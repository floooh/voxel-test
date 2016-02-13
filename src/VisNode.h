#pragma once
//------------------------------------------------------------------------------
/**
    @class VisNode
    @brief a node in the VisTree
*/
#include "Core/Types.h"

class VisNode {
public:
    enum Flags {
        GeomPending = (1<<0),   // geom is currently prepared for drawing
        OutOfBounds = (1<<1)   // entire node is below min-height or above max-height
    };
    static const int NumGeoms = 3;
    static const int NumChilds = 4;
    Oryol::uint16 flags;
    Oryol::int16 geoms[NumGeoms];       // up to 3 geoms
    Oryol::int16 childs[NumChilds];     // 4 child nodes (or none)

    /// reset the node
    void Reset() {
        this->flags = 0;
        for (int i = 0; i < NumGeoms; i++) {
            this->geoms[i] = Oryol::InvalidIndex;
        }
        for (int i = 0; i < NumChilds; i++) {
            this->childs[i] = Oryol::InvalidIndex;
        }
    }
    /// return true if this is a leaf node
    bool IsLeaf() const {
        return Oryol::InvalidIndex == this->childs[0];
    }
    /// return true if has node has a draw geom assigned
    bool HasGeom() const {
        return Oryol::InvalidIndex != this->geoms[0];
    }
    /// return true if the node needs geoms to be generated
    bool NeedsGeom() const {
        return (Oryol::InvalidIndex == this->geoms[0]) && !(this->flags & GeomPending);
    }
    /// return true if node is waiting for geom
    bool WaitsForGeom() const {
        return this->flags & GeomPending;
    }
    /// return true if out-of-bounds flag is set
    bool IsOutOfBounds() const {
        return this->flags & OutOfBounds;
    }
};