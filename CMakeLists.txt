cmake_minimum_required(VERSION 3.20)
project(search_system VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/)

add_subdirectory(${PROJECT_SOURCE_DIR}/SSHelp)
add_subdirectory(${PROJECT_SOURCE_DIR}/SSParserConfig)
add_subdirectory(${PROJECT_SOURCE_DIR}/SSDataBase)
add_subdirectory(${PROJECT_SOURCE_DIR}/SSNetworking)
add_subdirectory(${PROJECT_SOURCE_DIR}/SSClient)
add_subdirectory(${PROJECT_SOURCE_DIR}/SSServer)