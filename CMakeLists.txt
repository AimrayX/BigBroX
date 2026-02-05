cmake_minimum_required(VERSION 3.15)
project(BigBroX VERSION 1.0 LANGUAGES CXX)

# 1. Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 2. Compiler Flags Setup
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # --- COMMON FLAGS ---
    add_compile_options(-march=native)
    add_compile_options(-Wall -Wextra)

    # --- RELEASE FLAGS ---
    # Only apply these when building in Release mode
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        add_compile_options(-funroll-loops)
        add_compile_definitions(NDEBUG)
    endif()

    # --- DEBUG FLAGS ---
    # Only apply these when building in Debug mode
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-O0)
        add_compile_options(-g)
        add_compile_options(-ggdb)
    endif()
endif()

# 4. Sources
set(SOURCES
    main.cpp
    engine.cpp
    position.cpp
    transposition.cpp
    attack.cpp
    utils.cpp
    UCIHandler.cpp
    game.cpp
    magicBitboards.cpp
)

add_executable(chess_engine ${SOURCES})

# 5. Linker Optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Only strip symbols (-s) in Release mode. 
    # Stripping removes the names of variables/functions, making debugging impossible.
    target_link_options(chess_engine PRIVATE $<$<CONFIG:Release>:-s>)
endif()
