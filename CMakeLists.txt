cmake_minimum_required(VERSION 3.22)

project(Calgine LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Shared Library Configuration ----
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

include(FetchContent)

# ---- Fetch glad (glad2) ----
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.6
)

# ---- Fetch eventpp (c++ event system) ----
FetchContent_Declare(
    eventpp
    GIT_REPOSITORY https://github.com/wqking/eventpp
    GIT_TAG        d2db8af2a46c79f8dc75759019fba487948e9442 # v0.1.3
)

FetchContent_MakeAvailable(glad)
FetchContent_MakeAvailable(eventpp)

if(NOT COMMAND glad_add_library)
    add_subdirectory(${glad_SOURCE_DIR}/cmake ${glad_BINARY_DIR}/glad_cmake EXCLUDE_FROM_ALL)
endif()

glad_add_library(glad
    REPRODUCIBLE
    LOADER
    API gl:core=4.6
)

# ---- Dependencies ----

if(NOT TARGET spdlog)
    # Stand-alone build
    find_package(spdlog REQUIRED)
endif()
find_package(OpenGL REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

# ---- C++ Standard ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Warnings ----
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)

# ---- Library ----
file(GLOB_RECURSE CALGINE_SOURCES CONFIGURE_DEPENDS src/*.cxx)

add_library(Calgine SHARED ${CALGINE_SOURCES})

target_compile_definitions(Calgine PRIVATE CALGINE_EXPORTS)

target_link_libraries(Calgine
    PUBLIC
        glad
        OpenGL::GL
        SDL3::SDL3
        spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>
        eventpp
    PRIVATE
        project_warnings
)

target_include_directories(Calgine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_precompile_headers(Calgine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/calgine_pch.h)

# ---- Documentation (Doxygen) ----
find_package(Doxygen QUIET)

if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
else()
    message(STATUS "Doxygen not found — 'docs' target will be unavailable")
endif()
