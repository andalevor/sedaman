set(PROJ_NAME sedaman)
project(${PROJ_NAME})
cmake_minimum_required(VERSION 3.1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 14)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SOURCES "src/*.cpp")

add_library(${PROJ_NAME} SHARED ${SOURCES})

if(MSVC)
	set(CMAKE_CXX_FLAGS_REALESE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
	target_compile_options(${PROJ_NAME} PRIVATE /W4 /WX)
else()
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fsanitize=address -g")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")
	target_compile_options(${PROJ_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory(tests)
