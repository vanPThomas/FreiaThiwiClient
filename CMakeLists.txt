cmake_minimum_required(VERSION 3.16)
project(FreiaThiwiClient VERSION 0.1.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- version -------------------------------------------------
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 2)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")
# -------------------------------------------------------------

# 𐍆𐍂𐌴𐌹𐌰 𐌸𐌹𐍅𐌹  Client - Free Servant Chat Client
add_executable(freia-thiwi-client
    src/main.cpp
    src/ClientConnect.cpp

    # ImGui core
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_tables.cpp

    # ImGui backends
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp

)

# Include directories for our code + ImGui
target_include_directories(freia-thiwi-client PRIVATE
    include
    imgui
    imgui/backends
)

# Find system libs
find_package(PkgConfig REQUIRED)

# GLFW
pkg_search_module(GLFW REQUIRED glfw3)
target_link_libraries(freia-thiwi-client ${GLFW_LIBRARIES})
target_include_directories(freia-thiwi-client PRIVATE ${GLFW_INCLUDE_DIRS})

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(freia-thiwi-client OpenGL::GL)

# Linux defaults (dl, pthread)
target_link_libraries(freia-thiwi-client dl pthread)

# Output to bin/
set_target_properties(freia-thiwi-client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
)

message("
𐍆𐍂𐌴𐌹𐌰 𐌸𐌹𐍅𐌹 Client v${PROJECT_VERSION}
Lightweight ImGui interface for the ultimate privacy.
Runs on potatoes.
")
