cmake_minimum_required(VERSION 3.20)
project(search_system VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin/libs)

#find_package(Boost REQUIRED)
#set(Boost_USE_STATIC_LIBS OFF) 
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)

#set(PostgreSQL_ADDITIONAL_VERSIONS "15")
#add_subdirectory(${PROJECT_SOURCE_DIR}/lib/libpqxx libpqxx-build)

IF(Boost_FOUND)
    add_subdirectory(${PROJECT_SOURCE_DIR}/http-client)
    add_subdirectory(${PROJECT_SOURCE_DIR}/http-server)
ELSEIF()
    message(FATAL_ERROR "Could not find Boost")
ENDIF()
