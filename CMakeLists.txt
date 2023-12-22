cmake_minimum_required(VERSION 3.27)

message(STATUS "Preparing Env...")

#Using Cpp-23
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_VERSION EQUAL 3.25)
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")
elseif (CMAKE_VERSION EQUAL 3.26)
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")
elseif (CMAKE_VERSION EQUAL 3.27)
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")
else()
    message(WARNING "Newer CMake version, CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API may not be set.")
endif()

project(GameEngine
    VERSION 0.0.1
)

message(STATUS "Type: ${CMAKE_BUILD_TYPE}")

message(STATUS "Using Generator : ${CMAKE_CXX_COMPILER_ID}")

set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"MSVCRT.lib\")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DDEBUG_LOCAL)
    message(STATUS "Debug Mode")
else()
    message(STATUS "Options: ${CMAKE_CXX_FLAGS_RELEASE}")
    message(STATUS "Release Mode")
endif ()

set(SRC_DIR src)
set(RES_DIR resource)
set(REF_DIR include)
set(LIB_DIR lib)

add_compile_definitions(ASSETS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}\")

add_compile_definitions(APPLICATION_NAME=\"Game_Engine_Demo-${PROJECT_VERSION}\")

message(STATUS "Loading...")

# Source Files...
file(GLOB_RECURSE SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.cpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.c
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.ui
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.qrc
)

# Header Files...
file(GLOB_RECURSE CURRENT_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.hpp
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h
#        ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp
)


# Modules...
#
file(GLOB_RECURSE FAST_MODULES
    ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.ixx
#    ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.cppm
)

message(STATUS "Linking...")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${REF_DIR})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/${LIB_DIR})

# Static link
link_libraries(
    freetype.lib
    glfw3.lib
    opengl32.lib
    gdi32.lib
    shell32.lib
    User32.lib
)

message(STATUS "Building...")

# Build .exe
add_executable(
        ${PROJECT_NAME}      #[[Project Name]]
        ${SOURCE_FILES}      #[[.cpp]]
        ${CURRENT_HEADERS}   #[[.h]]
        glad.c               #[[Glad Attachment]]
        main.cpp
)

target_sources(${PROJECT_NAME}
        PUBLIC
        ${FAST_MODULES}      #[[Modules]]
)

message(STATUS "Compile Complete...")


#if(0#[[CMAKE_CXX_COMPILER_ID STREQUAL "GNU"]])
#
#    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules-ts")
#    set(CMAKE_EXPERIMENTAL_CXX_MODULE_DYNDEP ON)
#
#    #    string(CONCAT CMAKE_EXPERIMENTAL_CXX_SCANDEP_SOURCE
#    #            "<CMAKE_CXX_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -E -x c++ <SOURCE>"
#    #            " -MT <DYNDEP_FILE> -MD -MF <DEP_FILE>"
#    #            " -fmodules-ts -fdep-file=<DYNDEP_FILE> -fdep-output=<OBJECT>"
#    #            " -fdep-format=trtbd")
#
#    #    set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG
#    #            " -fmodules-ts -fmodule-mapper=<MODULE_MAP_FILE>"
#    #            " -fdep-format=trtbd -x c++")
#
#    #    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fmodules-ts")
#
#    message(STATUS "Flags: ${CMAKE_CXX_FLAGS}")
#endif()