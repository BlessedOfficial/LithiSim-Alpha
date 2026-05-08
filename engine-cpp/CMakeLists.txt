cmake_minimum_required(VERSION 3.16)
project(LithiSimEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(lithisim_core
    src/csv_simulation_logger.cpp
    src/math_models.cpp
    src/simulate.cpp
    src/truck.cpp
)

target_include_directories(lithisim_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(lithisim
    src/main.cpp
)
target_link_libraries(lithisim PRIVATE lithisim_core)

if(MSVC)
    target_compile_options(lithisim_core PRIVATE /W4 /permissive-)
    target_compile_options(lithisim PRIVATE /W4 /permissive-)
else()
    target_compile_options(lithisim_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(lithisim PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.7.1
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(lithisim_tests
        tests/simulation_tests.cpp
    )
    target_link_libraries(lithisim_tests PRIVATE
        Catch2::Catch2WithMain
        lithisim_core
    )
    target_include_directories(lithisim_tests PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )

    include(Catch)
    catch_discover_tests(lithisim_tests)
endif()
