fips_begin_app(VoxelTest windowed)
    fips_files(
        Main.cc
        Volume.h
        VoxelGenerator.h VoxelGenerator.cc
        Geom.h Geom.cc
        GeomPool.h GeomPool.cc
        GeomMesher.h GeomMesher.cc
        VisMap.h VisMap.cc
        stb_voxel_render.h)
    oryol_shader(shaders.shd)
    fips_deps(Gfx Time Dbg)
fips_end_app()

# silence some stb_voxel_render warnings
if (FIPS_CLANG)
    set_target_properties(VoxelTest 
        PROPERTIES COMPILE_FLAGS 
        "-Wno-missing-field-initializers -Wno-unused-variable")
endif()
