cmake_minimum_required(VERSION 3.10)

project(FMCL)

add_compile_options(-O3 -g -march=native -flto -funroll-loops -ffast-math -ftree-vectorize -fopt-info-vec)

set(SOURCES src/main.c src/utility.h src/utility.c src/life.h src/life.c src/grid.h src/grid.c src/graphics.h src/graphics.c src/input.h src/input.c src/init.h src/init.c)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

add_executable(FMCL ${SOURCES})

set(CMAKE_BUILD_TYPE Release)

option(ENABLE_PROFILING "Enable profiling with gprof" ON)

if(ENABLE_PROFILING)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif()

target_link_libraries(FMCL SDL2 SDL2_image SDL2_mixer)

if( supported )
    message(STATUS "IPO / LTO enabled")
    set_property(TARGET FMCL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO / LTO not supported: <${error}>")
endif()
