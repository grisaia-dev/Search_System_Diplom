cmake_minimum_required(VERSION 3.20)
project(SSDataBase VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PostgreSQL_ADDITIONAL_VERSIONS "15")

if (WIN32)
	set(libpqxx_DIR D:/libraries/libpqxxx/lib/cmake/libpqxx)
	#set(libpqxx_INCLUDE_DIR D:/libraries/libpqxxx/include)
	#set(libpqxx_LIBRARY D:/libraries/libpqxxx/lib)
elseif(UNIX)
	set(pqxx_DIR /Users/jatex/Libraries/libspqxx)
	#set(pqxx_INCLUDE_DIR /Users/jatex/Libraries/libspqxx/include)
	#set(pqxx_LIBRARY /Users/jatex/Libraries/libspqxx/lib)
	set(PostgreSQL_ROOT /opt/homebrew/opt/postgresql@15)
endif()

find_package(libpqxx REQUIRED)
set(BUILD_SHARED_LIBS ON)

set(SOURCE_F
    ${PROJECT_SOURCE_DIR}/src/db.cpp
)

set(HEADER_F
    ${PROJECT_SOURCE_DIR}/include/SSDataBase/db.hpp
)

if (libpqxx_FOUND)
    add_library(${PROJECT_NAME} SHARED ${SOURCE_F} ${HEADER_F})

    target_include_directories(${PROJECT_NAME}
    	PUBLIC
    		$<INSTALL_INTERFACE:include>
    		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            libpqxx::pqxx
    	PRIVATE
    		SSHelp
    )

    target_link_libraries(${PROJECT_NAME}
    	PRIVATE
            libpqxx::pqxx
    		SSHelp
    )
endif()
