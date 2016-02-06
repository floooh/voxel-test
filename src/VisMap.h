#pragma once
//------------------------------------------------------------------------------
/**
    @class VisMap
    @brief visibility and LOD mip-map
    
    Keeps track of chunk visibility and level-of-detail. This is basically
    a quad-tree where the quad-tree-levels are layed out in mip-map-like
    2D arrays.
*/
#include "Core/Types.h"
#include "Config.h"
#include "glm/vec3.hpp"

class VisMap {
public:
    /// a visibility chunk
    struct Chunk {
        enum Flags : Oryol::int16 {
            None = 0,

            // view culling flags
            CullInside = (1<<0),    // the entire item is inside view volume
            CullOutside = (1<<1),   // the entire item is outside the view volume
            CullMask = (CullInside|CullOutside),

            // geom flags
            GeomValid = (1<<2),     // referenced geom is valid
        } flags = Flags::None;

        // draw-geom index
        Oryol::int16 GeomIndex = Oryol::InvalidIndex;
    };

    /// start index of mipmap levels
    int LevelIndex[Config::NumLevels];

    /// flat array of all mips of items, the size is rounded up to hold all mip maps
    static const int MaxNumChunks = (Config::MapDimChunks*Config::MapDimChunks) + (Config::MapDimChunks*Config::MapDimChunks)/2;
    Chunk Chunks[MaxNumChunks];

    /// initialize the VisMap object
    void Setup();
    /// compute index of chunk by address
    int IndexOf(int lvl, int x, int y) const;
    /// access chunk at address (read/write)
    Chunk& At(int lvl, int x, int y);
    /// access chunk at address (read-only)
    const Chunk& At(int lvl, int x, int y) const;
    /// get the bounding coordinates of a chunk given by address
    void Bounds(int lvl, int x, int y, int& outX0, int& outX1, int& outY0, int& outY1) const;
    /// compute translation vector for a bounds origin
    glm::vec3 Translation(int x0, int y0);
    /// compute scale vector for a bounds rect
    glm::vec3 Scale(int x0, int x1, int y0, int y1);
};
