cmake_minimum_required(VERSION 3.20)
project(SSSpider VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADER_F
	${PROJECT_SOURCE_DIR}/include/spider.hpp
)

if(WIN32)
	set(Boost_INCLUDE_DIR D:/libraries/Boost/include/boost-1_84)
	set(Boost_LIBRARY D:/libraries/Boost/lib)
endif()

if(UNIX)
    set(Boost_INCLUDE_DIR /Users/jatex/Libraries/boost/include)
    set(Boost_LIBRARY /Users/jatex/Libraries/boost/lib)
endif()

find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)

if (OpenSSL_FOUND)
	add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cpp ${HEADER_F})
	target_include_directories(${PROJECT_NAME}
		PUBLIC
			OpenSSL::SSL
			SSHelp
			SSDatabase
			SSParserConfig
			SSNetworking
			${Boost_INCLUDE_DIRS}
	)

	target_link_libraries(${PROJECT_NAME}
		PRIVATE
			OpenSSL::SSL
			SSHelp
			SSDataBase
			SSParserConfig
			SSNetworking
			${Boost_LIBRARIES}
	)
endif()
