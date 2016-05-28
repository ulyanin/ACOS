cmake_minimum_required(VERSION 3.0)
project(life_game)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c89 -Wall")

set(SOURCE_FILES life_game_server.cpp node.cpp)
add_executable(life_game_server ${SOURCE_FILES})

set(SOURCE_FILES life_game_node.cpp node.cpp)
add_executable(life_game_node ${SOURCE_FILES})

set(SOURCE_FILES node_spawner.cpp)
add_executable(life_game_node_spawner ${SOURCE_FILES})


set(SOURCE_FILES_LIFE_GAME life.c)
add_executable(life_game_simply ${SOURCE_FILES_LIFE_GAME})

set(CMAKE_C_FLAGS "-pthread -Wall")
set(SOURCE_FILES_LIFE_GAME_THREADS life_game_threads.c)
add_executable(life_game_threads ${SOURCE_FILES_LIFE_GAME_THREADS})
target_link_libraries(life_game_threads ${CMAKE_DL_LIBS})
