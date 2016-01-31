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
    static const int NumGeoms = 64;

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

private:
    Oryol::Id indexMesh;
    Oryol::StaticArray<Geom, NumGeoms> geoms;
    Oryol::Array<int> freeGeoms;
};

//------------------------------------------------------------------------------
inline int
GeomPool::Alloc() {
    return this->freeGeoms.PopBack();
}

//------------------------------------------------------------------------------
inline void
GeomPool::Free(int index) {
    this->freeGeoms.Add(index);
}

//------------------------------------------------------------------------------
inline Geom&
GeomPool::GeomAt(int index) {
    return this->geoms[index];
}

//------------------------------------------------------------------------------
inline const Geom&
GeomPool::GeomAt(int index) const {
    return this->geoms[index];
}
