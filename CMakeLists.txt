cmake_minimum_required(VERSION 3.8)
project(buckshot-roulette)

add_executable(${PROJECT_NAME} src/main.cc src/expectimax.cc src/item_manager.cc src/transposition_table.cc)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)
