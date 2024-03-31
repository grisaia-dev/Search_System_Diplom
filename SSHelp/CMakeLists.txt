cmake_minimum_required(VERSION 3.20)
project(SSHelp VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADER_F
	${PROJECT_SOURCE_DIR}/include/SSHelp/hit.hpp
)

set(SOURCE_F
	${PROJECT_SOURCE_DIR}/src/hit.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_F} ${HEADER_F})

target_include_directories(${PROJECT_NAME}
	PUBLIC
		$<INSTALL_INTERFACE:include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE
)