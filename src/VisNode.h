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
        CullInside = (1<<1),    // fully inside view volume
        CullOutside = (1<<2),   // fully outside view volume
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
    };
    /// return true if this is a leaf node
    bool IsLeaf() const {
        return Oryol::InvalidIndex == this->childs[0];
    };
};