cmake_minimum_required(VERSION 3.20)
project(SSServer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cpp)

target_include_directories(${PROJECT_NAME}
	PUBLIC
		SSParserConfig
		SSNetworking
)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		SSParserConfig
		SSNetworking
)