set(PROJ_NAME sedaman)
project(${PROJ_NAME})
cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB SOURCES "src/*.cpp")

add_library(${PROJ_NAME} SHARED ${SOURCES})
#add_library(${PROJ_NAME}_static STATIC ${SOURCES})

if(MSVC)
        target_compile_options(${PROJ_NAME} PRIVATE /W4 /WX)
else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fsanitize=address -g")
        target_compile_options(${PROJ_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory(tests)
