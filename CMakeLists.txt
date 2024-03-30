cmake_minimum_required(VERSION 3.20)
project(search_system VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(build_dir ${CMAKE_SOURCE_DIR}/build)
#set(CMAKE_BUILD_DIRECTORY ${build_dir})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${build_dir})
#set(CMAKE_BUILD_FILES_DIRECTORY ${build_dir})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/libs)

#find_package(Boost REQUIRED)
#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)

#set(PostgreSQL_ADDITIONAL_VERSIONS "15")
#add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libpqxx libpqxx-build)

#IF(Boost_FOUND)
    add_subdirectory(${PROJECT_SOURCE_DIR}/SSNetworking)
    add_subdirectory(${PROJECT_SOURCE_DIR}/SSClient)
    add_subdirectory(${PROJECT_SOURCE_DIR}/SSServer)
#ELSEIF()
#    message(FATAL_ERROR "Could not find Boost")
#ENDIF()
