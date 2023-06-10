set(PROJ_NAME sedaman)
cmake_policy(SET CMP0048 NEW)
project(${PROJ_NAME} VERSION 0.0.1 LANGUAGES CXX)
cmake_minimum_required(VERSION 3.4...3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/extern/pybind11/include)
file(GLOB SOURCES "src/*.cpp")

add_subdirectory(extern/pybind11)
pybind11_add_module(pysedaman ${SOURCES} "src/pybind/pybind.cpp")
add_library(${PROJ_NAME} SHARED ${SOURCES})
add_library(${PROJ_NAME}_static STATIC ${SOURCES})
target_link_libraries(${PROJ_NAME} ${PYTHON_LIBRARIES})
target_link_libraries(${PROJ_NAME}_static ${PYTHON_LIBRARIES})

if(MSVC)
        target_compile_options(${PROJ_NAME} PRIVATE /W4 /WX)
else()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined -fsanitize=address -g")
        target_compile_options(${PROJ_NAME} PRIVATE -Wall -Wextra -pedantic -Werror)
endif()

add_subdirectory(tests)
