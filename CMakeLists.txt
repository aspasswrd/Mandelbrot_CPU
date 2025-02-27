cmake_minimum_required(VERSION 3.30)
project(mandelbrot)

set(CMAKE_CXX_STANDARD 26)

# Опция для выбора версии
set(VERSION "DEFAULT" CACHE STRING "Выбор версии (DEFAULT|AVX|HIGH_PRECISION)")

# Общие зависимости
find_package(SDL2 REQUIRED)
find_package(OpenMP REQUIRED)

# Общие настройки
add_executable(mandelbrot)
target_include_directories(mandelbrot PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(mandelbrot PRIVATE ${SDL2_LIBRARIES} OpenMP::OpenMP_CXX)

# Общие флаги оптимизации
target_compile_options(mandelbrot PRIVATE -O3 -ffast-math -funroll-loops)

# Настройки для разных версий
if(VERSION STREQUAL "DEFAULT")
    message(STATUS "Сборка стандартной версии")
    target_sources(mandelbrot PRIVATE mandelbrot.cpp)
    target_compile_options(mandelbrot PRIVATE -ftree-vectorize)

elseif(VERSION STREQUAL "AVX")
    message(STATUS "Сборка AVX-оптимизированной версии")
    target_sources(mandelbrot PRIVATE mandelbrot_AVX.cpp)
    target_compile_options(mandelbrot PRIVATE
            -mavx2 -msse4.2 -mfma -march=native
    )

elseif(VERSION STREQUAL "HIGH_PRECISION")
    message(STATUS "Сборка высокоточной версии с Boost")
    target_sources(mandelbrot PRIVATE mandelbrot_high_precision.cpp)
    find_package(Boost REQUIRED)
    target_include_directories(mandelbrot PRIVATE ${Boost_INCLUDE_DIRS})
    target_compile_definitions(mandelbrot PRIVATE BOOST_ALL_NO_LIB)

else()
    message(FATAL_ERROR "Неизвестная версия: ${VERSION}. Допустимые значения: DEFAULT|AVX|HIGH_PRECISION")
endif()

# Общие флаги для всех версий
target_compile_options(mandelbrot PRIVATE ${OpenMP_CXX_FLAGS})
target_link_options(mandelbrot PRIVATE ${OpenMP_CXX_FLAGS})
