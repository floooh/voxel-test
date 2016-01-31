fips_begin_app(VoxelTest windowed)
    fips_files(
        Main.cc
        Volume.h
        Geom.h Geom.cc
        GeomPool.h GeomPool.cc
        GeomMesher.h GeomMesher.cc
        stb_voxel_render.h)
    oryol_shader(shaders.shd)
    fips_deps(Gfx Time)
fips_end_app()
