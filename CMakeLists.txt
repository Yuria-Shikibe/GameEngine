cmake_minimum_required(VERSION 3.26)

message(STATUS "Preparing Env...")

#Using Cpp-23
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(GameEngine
    VERSION 0.0.1
)

message(STATUS "Type: ${CMAKE_BUILD_TYPE}")

message(STATUS "Using Generator : ${CMAKE_CXX_COMPILER_ID}")

#find msvc dir, totally mess
set(MSVC_DIR ${CMAKE_CXX_COMPILER})
get_filename_component(MSVC_DIR ${MSVC_DIR} DIRECTORY)
get_filename_component(MSVC_DIR ${MSVC_DIR} DIRECTORY)
get_filename_component(MSVC_DIR ${MSVC_DIR} DIRECTORY)
get_filename_component(MSVC_DIR ${MSVC_DIR} DIRECTORY)

message(STATUS "Msvc Current Dir: ${MSVC_DIR}")

set(MSVC_MODULE_DIR ${MSVC_DIR}/modules)
file(COPY ${MSVC_MODULE_DIR} DESTINATION
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SRC_DIR src)
set(RES_DIR properties)
set(REF_DIR include)
set(LIB_DIR lib)
set(BUILD_DIR build)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${BUILD_DIR})

set(EXE_DIR_RELA ${BUILD_DIR}/${CMAKE_BUILD_TYPE})
set(EXE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${EXE_DIR_RELA})

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    message(STATUS "Debug Mode")
    set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:library)
    add_compile_definitions(ASSETS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}\")

    file(GLOB_RECURSE BIN
            ${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}/*.dll
    )

    file(COPY ${BIN} DESTINATION
            ${EXE_DIR}
    )
else()
    message(STATUS "CXX Options: ${CMAKE_CXX_FLAGS_RELEASE}")
    message(STATUS "Release Mode")
endif ()


add_compile_definitions(APPLICATION_NAME=\"Game_Engine_Demo-${PROJECT_VERSION}\")

message(STATUS "Loading...")

# Source Files...
file(GLOB_RECURSE SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.cpp
)

# Header Files...
file(GLOB_RECURSE CURRENT_HEADERS
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.hpp
)


# Modules...
#
file(GLOB_RECURSE FAST_MODULES
    ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.cppm
    ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR}/*.ixx
)

message(STATUS "Linking...")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/${REF_DIR})

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/${LIB_DIR})

# Static link
link_libraries(
    freetype.lib
    glfw3.lib
    irrKlang.lib
#    opengl32.lib
#    gdi32.lib
#    shell32.lib
#    User32.lib
#    irrKlang.exp
)

#macro(get_WIN32_WINNT version)
#
#message(STATUS "ON Windows")
link_libraries(
    dwmapi.lib
)
#endmacro()

message(STATUS "Building...")

# Build .exe
add_executable(
        ${PROJECT_NAME}      #[[Project Name]]
        ${SOURCE_FILES}      #[[.cpp]]
        ${CURRENT_HEADERS}   #[[.h]]
        glad.c               #[[Glad Attachment]]
        main.cpp
#        test.cpp
)

target_sources(${PROJECT_NAME}
        PUBLIC
        modules/std.compat.ixx
        modules/std.ixx
        ${FAST_MODULES}      #[[Modules]]
        ${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}/target.rc
)

set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
#target_compile_options(${PROJECT_NAME} PRIVATE "/MD$<$<CONFIG:Debug>:d>")

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(FILE_TO_DELETE "${EXE_DIR}/${PROJECT_NAME}.pdb")
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove ${FILE_TO_DELETE}
        COMMENT "Deleting ${FILE_TO_DELETE}"
    )
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    message(STATUS "Copy Properties Setup")

    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}/ ${EXE_DIR}
    )
endif ()


message(STATUS "Configuration Complete...")
