fips_begin_app(VoxelTest windowed)
    fips_files(
        Main.cc
        Volume.h Config.h
        VoxelGenerator.h VoxelGenerator.cc
        GeomPool.h GeomPool.cc
        GeomMesher.h GeomMesher.cc
        VisNode.h VisBounds.h
        VisTree.h VisTree.cc
        Camera.h Camera.cc
        stb_voxel_render.h)
    oryol_shader(shaders.shd)
    fips_deps(Gfx Input Dbg)
fips_end_app()

# silence some stb_voxel_render warnings
if (FIPS_CLANG)
    set_target_properties(VoxelTest 
        PROPERTIES COMPILE_FLAGS 
        "-Wno-missing-field-initializers -Wno-unused-variable")
endif()
