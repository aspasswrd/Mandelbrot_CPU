cmake_minimum_required(VERSION 3.30)
project(mandelbrot)

set(CMAKE_CXX_STANDARD 26)

find_package(SDL2 REQUIRED)
find_package(OpenMP REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -ffast-math -lSDL2")

# Добавляем исходный файл
add_executable(mandelbrot mandelbrot.cpp)

target_compile_options(mandelbrot PRIVATE ${OpenMP_CXX_FLAGS})
target_link_libraries(mandelbrot PRIVATE OpenMP::OpenMP_CXX)

target_include_directories(mandelbrot PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(mandelbrot PRIVATE ${SDL2_LIBRARIES})
