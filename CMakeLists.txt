cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

# Use C++ 20 as requested
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Create the executable named 'chess_engine'
# Add ALL your .cpp files here
add_executable(chess_engine
    main.cpp       # Assuming you have a main.cpp
    attack.cpp
    attack.hpp
    position.cpp
    position.hpp
    engine.cpp
    engine.hpp
    game.cpp
    game.hpp
    UCIHandler.cpp
    UCIHandler.hpp
    utils.cpp
    utils.hpp
    types.hpp
)

target_include_directories(chess_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
