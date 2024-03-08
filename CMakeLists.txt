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
    set(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"MSVCRT.lib\")
    add_compile_definitions(ASSETS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}\")
    
    file(GLOB_RECURSE BIN
            ${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}/*.dll
    )

    file(COPY ${BIN} DESTINATION
            ${EXE_DIR}
    )
else()
    message(STATUS "Options: ${CMAKE_CXX_FLAGS_RELEASE}")
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

message(STATUS "Building...")

# Build .exe
add_executable(
        ${PROJECT_NAME}      #[[Project Name]]
        ${SOURCE_FILES}      #[[.cpp]]
        ${CURRENT_HEADERS}   #[[.h]]
        glad.c               #[[Glad Attachment]]
        main.cppm
)

target_sources(${PROJECT_NAME}
        PUBLIC
        ${FAST_MODULES}      #[[Modules]]
)

if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
    message(STATUS "Copy Properties Setup")

    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}/ ${EXE_DIR}
    )
endif ()


message(STATUS "Compile Complete...")
