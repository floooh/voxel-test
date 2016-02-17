#pragma once
//------------------------------------------------------------------------------
/**
    @class GeomPool
    @brief a pool of reusable voxel meshes
*/
#include "Geom.h"
#include "Volume.h"
#include "Gfx/Setup/GfxSetup.h"
#include "Core/Containers/StaticArray.h"
#include "Core/Containers/Array.h"

class GeomPool {
public:
    static const int NumGeoms = 700;

    /// initialize the geom pool
    void Setup(const Oryol::GfxSetup& gfxSetup);
    /// discard the geom pool
    void Discard();

    /// alloc a new geom, return geom index
    int Alloc();
    /// free a geom
    void Free(int index);
    /// free all geoms
    void FreeAll();
    /// get geom by index (read/write)
    Geom& GeomAt(int index);
    /// get geom by index (read-only)
    const Geom& GeomAt(int index) const;

    Oryol::Id indexMesh;
    Oryol::StaticArray<Geom, NumGeoms> geoms;
    Oryol::Array<int> freeGeoms;
};

//------------------------------------------------------------------------------
inline int
GeomPool::Alloc() {
    o_assert(!this->freeGeoms.Empty());
    int index = this->freeGeoms.PopBack();
    o_assert(Oryol::InvalidIndex != index);
    return index;
}

//------------------------------------------------------------------------------
inline void
GeomPool::Free(int index) {
    o_assert_dbg(Oryol::InvalidIndex != index);
    this->freeGeoms.Add(index);
}

//------------------------------------------------------------------------------
inline Geom&
GeomPool::GeomAt(int index) {
    o_assert_dbg(Oryol::InvalidIndex != index);
    return this->geoms[index];
}

//------------------------------------------------------------------------------
inline const Geom&
GeomPool::GeomAt(int index) const {
    return this->geoms[index];
}