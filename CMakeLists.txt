cmake_minimum_required(VERSION 3.22)

project(Calgine LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---- Shared Library Configuration ----
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---- Options for using system libs or vendoring ----
option(USE_SYSTEM_SPDLOG "If ON, try to use a system-installed spdlog via find_package(spdlog). If not found, fall back to vendoring." OFF)
option(USE_SYSTEM_SDL3  "If ON, try to use a system-installed SDL3 via find_package(SDL3). If not found, fall back to vendoring." OFF)

include(FetchContent)

# ---- Fetch glad (glad2) ----
FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.6
)
FetchContent_MakeAvailable(glad)

# If the glad helper macros are not installed by FetchContent, add the cmake subdir directly
if(NOT COMMAND glad_add_library)
    add_subdirectory(${glad_SOURCE_DIR}/cmake ${glad_BINARY_DIR}/glad_cmake EXCLUDE_FROM_ALL)
endif()

glad_add_library(glad
    REPRODUCIBLE
    LOADER
    API gl:core=4.6
)

# ---- Fetch ImGui ----
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# Create ImGui library target
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui
    PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui
    PUBLIC
        SDL3::SDL3
        OpenGL::GL
)

# ---- Fetch GLM (header-only math library) ----
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.0
)
FetchContent_MakeAvailable(glm)

# ---- spdlog: prefer system if requested, otherwise use FetchContent ----
if(USE_SYSTEM_SPDLOG)
    find_package(spdlog CONFIG QUIET)
endif()

if(NOT TARGET spdlog::spdlog)
    set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    FetchContent_MakeAvailable(spdlog)
endif()

# ---- SDL3: prefer system if requested, otherwise fetch via FetchContent ----
if(USE_SYSTEM_SDL3)
    find_package(SDL3 CONFIG QUIET)
endif()

if(NOT TARGET SDL3::SDL3)
    # If you have a submodule copy in src/vendor/SDL or src/vendor/SDL3, prefer that:
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/SDL/CMakeLists.txt" OR EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/SDL3/CMakeLists.txt")
        # Some repos keep SDL in src/vendor/SDL; adjust if necessary
        set(SDL_TEST OFF CACHE BOOL "" FORCE)
        set(SDL_TESTS OFF CACHE BOOL "" FORCE)
        set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/vendor/SDL/CMakeLists.txt")
            add_subdirectory(src/vendor/SDL)
        else()
            add_subdirectory(src/vendor/SDL3)
        endif()
    else()
        # Fetch the official SDL repo (use a release tag appropriate to your needs)
        set(SDL_TEST OFF CACHE BOOL "" FORCE)
        set(SDL_TESTS OFF CACHE BOOL "" FORCE)
        set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(
            SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG ca537d651b82f28bc280685391665eb25c856e00
        )
        FetchContent_MakeAvailable(SDL3)
    endif()
endif()

# ---- OpenGL ----
# Keep OpenGL as a find_package since OpenGL is typically provided by the system/driver.
# Most systems provide an OpenGL config and the imported target OpenGL::GL
find_package(OpenGL REQUIRED)

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

# Remove "lib" prefix
set_target_properties(Calgine PROPERTIES
    PREFIX ""
)

target_compile_definitions(Calgine PRIVATE CALGINE_EXPORTS)

# Link libraries. spdlog::spdlog and SDL3::SDL3 will exist either because the system provided them
# or because we added them via add_subdirectory/FetchContent above.
target_link_libraries(Calgine
    PUBLIC
        glad
        OpenGL::GL
        SDL3::SDL3
        spdlog::spdlog
        $<$<BOOL:${MINGW}>:ws2_32>
        imgui
        glm::glm
        $<$<BOOL:${ENABLE_ASAN}>:asan_flags>
    PRIVATE
        project_warnings
)

target_include_directories(Calgine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Precompiled headers (keep as in original)
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

# ---- Helpful messages / diagnostics ----
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
message(STATUS "CMake build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Using system spdlog: ${USE_SYSTEM_SPDLOG}")
message(STATUS "Using system SDL3:  ${USE_SYSTEM_SDL3}")
if(TARGET spdlog::spdlog)
    message(STATUS "spdlog target available.")
else()
    message(WARNING "spdlog target NOT available. Build will likely fail.")
endif()
if(TARGET SDL3::SDL3)
    message(STATUS "SDL3 target available.")
else()
    message(WARNING "SDL3 target NOT available. Build will likely fail.")
endif()
if(TARGET imgui)
    message(STATUS "ImGui target available.")
else()
    message(WARNING "ImGui target NOT available. Build will likely fail.")
endif()
if(TARGET glm::glm)
    message(STATUS "GLM target available.")
else()
    message(WARNING "GLM target NOT available. Build will likely fail.")
endif()
