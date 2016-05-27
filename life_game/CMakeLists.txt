cmake_minimum_required(VERSION 3.0)
project(life_game)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

set(SOURCE_FILES life_game_server.cpp node.cpp)
add_executable(life_game_server ${SOURCE_FILES})


set(SOURCE_FILES life_game_node.cpp node.cpp)
add_executable(life_game_node ${SOURCE_FILES})