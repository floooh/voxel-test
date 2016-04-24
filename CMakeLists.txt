#
# project: voxel-test
#
cmake_minimum_required(VERSION 2.8)

# include the fips main cmake file
get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
include("${FIPS_ROOT_DIR}/cmake/fips.cmake")

# include_directories(src)

fips_setup()
fips_project(voxel-test)
fips_ide_group("Oryol")
fips_import_oryol_Core()
fips_import_oryol_Gfx()
fips_import_oryol_Input()
fips_import_oryol_Resource()
fips_import_oryol_Dbg()
fips_ide_group("Libs")
if (ORYOL_OPENGL)
    fips_import_fips_glfw_glfw3()
endif()
fips_ide_group("")
fips_add_subdirectory(src)
fips_finish()


