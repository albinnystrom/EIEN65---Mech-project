cmake_minimum_required(VERSION 3.14)
project(raygui_example C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ---------------------------------------------------------
# Fetch raylib from GitHub and build it as a subproject
# ---------------------------------------------------------
include(FetchContent)

# Don't build raylib's examples/games
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master    # or "5.5" if that tag exists on your clone
)

FetchContent_MakeAvailable(raylib)

# ---------------------------------------------------------
# Our executable
# ---------------------------------------------------------
add_executable(${PROJECT_NAME}
    src/main.c
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external   # for raygui.h
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        raylib
)

# ---------------------------------------------------------
# Platform-specific extra libs
# ---------------------------------------------------------
if(APPLE)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
    )
elseif(UNIX AND NOT APPLE)
    # Linux
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(X11 REQUIRED x11)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            m
            dl
            pthread
            ${X11_LIBRARIES}
    )
elseif(WIN32)
    # Windows
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            opengl32
            winmm
            gdi32
    )
endif()
