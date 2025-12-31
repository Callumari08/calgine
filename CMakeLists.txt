cmake_minimum_required(VERSION 3.22)

project(Calgine LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

# ---- Fetch glad (glad2) ----
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.6
)

FetchContent_MakeAvailable(glad)

if(NOT COMMAND glad_add_library)
    add_subdirectory(${glad_SOURCE_DIR}/cmake ${glad_BINARY_DIR}/glad_cmake EXCLUDE_FROM_ALL)
endif()

glad_add_library(glad
    REPRODUCIBLE
    LOADER
    API gl:core=4.6
)

# ---- Dependencies ----
find_package(OpenGL REQUIRED)
find_package(SDL3 CONFIG REQUIRED)

# ---- C++ Standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Warnings ----
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)

# ---- Library ----
file(GLOB_RECURSE CALGINE_SOURCES CONFIGURE_DEPENDS src/*.cxx)

add_library(Calgine ${CALGINE_SOURCES})

target_link_libraries(Calgine
    PUBLIC
        glad
        OpenGL::GL
        SDL3::SDL3
    PRIVATE
        project_warnings
)

target_include_directories(Calgine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

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
