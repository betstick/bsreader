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

option(DEBUG "enable verbose stuff and GDB support" OFF)

if(DEBUG)
	add_compile_definitions(DEBUG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb")
	message("Using debug mode...")
else()
	message("Using release...")
endif()
unset(DEBUG CACHE)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}__SOURCES} ${${PROJECT_NAME}__INCLUDES})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_link_libraries(${PROJECT_NAME} )