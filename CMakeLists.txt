cmake_minimum_required(VERSION 3.30)
project(mandelbrot)

set(CMAKE_CXX_STANDARD 26)

# Version selection
set(VERSION "DEFAULT" CACHE STRING "Build version (DEFAULT|AVX|HIGH_PRECISION)")

# Common dependencies
find_package(SDL2 REQUIRED)
find_package(OpenMP REQUIRED)

# Common settings
add_executable(mandelbrot)
target_include_directories(mandelbrot PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(mandelbrot PRIVATE ${SDL2_LIBRARIES} OpenMP::OpenMP_CXX)

# Portable optimization flags
target_compile_options(mandelbrot PRIVATE
        -O3
        -ffast-math
        -funroll-loops
        -ftree-vectorize
)

# Version-specific configurations
if(VERSION STREQUAL "DEFAULT")
    message(STATUS "Building default version")
    target_sources(mandelbrot PRIVATE mandelbrot.cpp)

elseif(VERSION STREQUAL "AVX")
    message(STATUS "Building AVX-optimized version")
    target_sources(mandelbrot PRIVATE mandelbrot_AVX.cpp)

    # AVX flags with compatibility check
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-mavx2 AVX2_SUPPORTED)
    if(AVX2_SUPPORTED)
        target_compile_options(mandelbrot PRIVATE -mavx2)
    else()
        message(WARNING "AVX2 not supported - using generic optimizations")
    endif()

elseif(VERSION STREQUAL "HIGH_PRECISION")
    message(STATUS "Building high-precision version with Boost")
    target_sources(mandelbrot PRIVATE mandelbrot_high_precision.cpp)
    find_package(Boost REQUIRED)
    target_include_directories(mandelbrot PRIVATE ${Boost_INCLUDE_DIRS})
    target_compile_definitions(mandelbrot PRIVATE BOOST_ALL_NO_LIB)

else()
    message(FATAL_ERROR "Unknown version: ${VERSION}. Valid options: DEFAULT|AVX|HIGH_PRECISION")
endif()

# Common flags for all versions
target_compile_options(mandelbrot PRIVATE ${OpenMP_CXX_FLAGS})
target_link_options(mandelbrot PRIVATE ${OpenMP_CXX_FLAGS})

# Portable architecture flags
if(NOT CMAKE_SYSTEM_PROCESSOR)
    set(CMAKE_SYSTEM_PROCESSOR "x86_64")
endif()

message(STATUS "Building for ${CMAKE_SYSTEM_PROCESSOR} architecture")
