cmake_minimum_required(VERSION 3.20)
project(SSNetworking VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

if(WIN32)
	set(Boost_INCLUDE_DIR D:/libraries/Boost/include/boost-1_84)
	set(Boost_LIBRARY D:/libraries/Boost/lib)
endif()

if(UNIX)
	set(Boost_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/lib/boost/unix/include)
	set(Boost_LIBRARY ${PROJECT_SOURCE_DIR}/lib/boost/unix/lib)
endif()

find_package(Boost REQUIRED)

set(SOURCE_F
	${PROJECT_SOURCE_DIR}/src/base.cpp
)

if(Boost_FOUND)
	add_library(${PROJECT_NAME} STATIC ${SOURCE_F})

	target_include_directories(${PROJECT_NAME}
		PUBLIC
			$<INSTALL_INTERFACE:include>
			$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		PRIVATE
			${Boost_INCLUDE_DIRS}
	)

	target_link_libraries(${PROJECT_NAME} 
		PRIVATE
			${Boost_LIBRARIES}
	)
endif()

