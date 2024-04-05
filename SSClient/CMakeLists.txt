cmake_minimum_required(VERSION 3.20)
project(SSSpider VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADER_F
	${PROJECT_SOURCE_DIR}/include/spider.hpp
)

#find_package(Boost REQUIRED)
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
	)

	target_link_libraries(${PROJECT_NAME}
		PRIVATE
			OpenSSL::SSL
			SSHelp
			SSDataBase
			SSParserConfig
			SSNetworking
	)
endif()
