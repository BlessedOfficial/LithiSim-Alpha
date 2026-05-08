cmake_minimum_required(VERSION 3.16)
project(LithiSimEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(lithisim
    src/csv_simulation_logger.cpp
    src/main.cpp
    src/math_models.cpp
    src/simulate.cpp
    src/truck.cpp
)

target_include_directories(lithisim PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

if(MSVC)
    target_compile_options(lithisim PRIVATE /W4 /permissive-)
else()
    target_compile_options(lithisim PRIVATE -Wall -Wextra -Wpedantic)
endif()
