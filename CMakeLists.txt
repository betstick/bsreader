cmake_minimum_required(VERSION 3.21)
#set(CMAKE_INSTALL_PREFIX /usr) #needed?
project(BSREADER)

message(STATUS ${PROJECT_SOURCE_DIR})

enable_language(CXX)
include(GNUInstallDirs)

macro(add_dir DIRS)
    foreach (dir ${DIRS})
        message("adding  ${dir} to ${PROJECT_NAME}")
        include_directories(${dir})
        file(GLOB ${dir}__INCLUDES_H ${dir} ${dir}/*.h)
        file(GLOB ${dir}__INCLUDES_HPP ${dir} ${dir}/*.hpp)
        list(APPEND ${PROJECT_NAME}__INCLUDES ${${dir}__INCLUDES_H} ${${dir}__INCLUDES_HPP})
        file(GLOB ${dir}__SOURCES_CPP ${dir} ${dir}/*.cpp ${dir}/*.cxx)
        file(GLOB ${dir}__SOURCES_C ${dir} ${dir}/*.c)
        list(APPEND ${PROJECT_NAME}__SOURCES ${${dir}__SOURCES_C} ${${dir}__SOURCES_CPP})
    endforeach()
endmacro()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_TYPE x64)
else (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCH_TYPE x86)
endif()

set(COMPILE_OUTPUT_FOLDER ${CMAKE_SOURCE_DIR}/bin/release/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${COMPILE_OUTPUT_FOLDER})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${COMPILE_OUTPUT_FOLDER})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${COMPILE_OUTPUT_FOLDER})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

list(APPEND DIRS
    "."
    )

add_dir("${DIRS}")

option(BUILD "enable verbose stuff and GDB support" OFF)

if(BUILD STREQUAL "debug")
	#enable GDB and disable all optimizations
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O0")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0")
	#show all types of errors, but limit to 8 at a time
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror  -fmax-errors=8")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror  -fmax-errors=8")
	#enable this for my debug code preprocessor stuff
	add_compile_definitions(DEBUG)
	#needed to prevent VMA werror stuff from stopping compile.
	add_compile_definitions(VMA_DEBUG_DETECT_CORRUPTION)
	add_compile_definitions(VMA_DEBUG_MARGIN=1)
	message(STATUS "Using debug mode...")
else()
	set(BUILD "release")
	#optimize aggressively
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
	message(STATUS "Using release mode...")
endif()
unset(BUILD CACHE)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}__SOURCES} ${${PROJECT_NAME}__INCLUDES})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME} )