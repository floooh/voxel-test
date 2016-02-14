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
    };
    static const Oryol::int16 InvalidGeom = -1;
    static const Oryol::int16 EmptyGeom = -2;
    static const Oryol::int16 InvalidChild = -1;
    static const int NumGeoms = 3;
    static const int NumChilds = 4;
    Oryol::uint16 flags;
    Oryol::int16 geoms[NumGeoms];       // up to 3 geoms
    Oryol::int16 childs[NumChilds];     // 4 child nodes (or none)

    /// reset the node
    void Reset() {
        this->flags = 0;
        for (int i = 0; i < NumGeoms; i++) {
            this->geoms[i] = InvalidGeom;
        }
        for (int i = 0; i < NumChilds; i++) {
            this->childs[i] = InvalidChild;
        }
    }
    /// return true if this is a leaf node
    bool IsLeaf() const {
        return InvalidChild == this->childs[0];
    }
    /// return true if has node has a draw geom assigned
    bool HasGeom() const {
        return InvalidGeom != this->geoms[0];
    }
    /// return true if the node is an empty volume (doesn't need drawing even if visible)
    bool HasEmptyGeom() const {
        return EmptyGeom == this->geoms[0];
    }
    /// return true if the node needs geoms to be generated
    bool NeedsGeom() const {
        return (InvalidGeom == this->geoms[0]) && !(this->flags & GeomPending);
    }
    /// return true if node is waiting for geom
    bool WaitsForGeom() const {
        return this->flags & GeomPending;
    }
};
