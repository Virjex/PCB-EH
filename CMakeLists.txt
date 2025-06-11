cmake_minimum_required(VERSION 3.15)
project(PCBEH)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Paths
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/dependencies/imgui)
set(GLFW_DIR ${CMAKE_SOURCE_DIR}/dependencies/GLFW)

# ImGui sources
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

# GLFW build
add_subdirectory(${GLFW_DIR} ${CMAKE_BINARY_DIR}/glfw)

# Vulkan
find_package(Vulkan REQUIRED)

# Auto find src/*.cpp and src/*.h
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
)

# Build executable
add_executable(cad-gui-vulkan
    ${SRC_FILES}
    ${IMGUI_SOURCES}
)

# Includes
target_include_directories(cad-gui-vulkan PRIVATE
    ${CMAKE_SOURCE_DIR}/src  # <--- ADD THIS — covers core, entity, gui, rendering, utils
    ${CMAKE_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${GLFW_DIR}/include
    ${Vulkan_INCLUDE_DIRS} # optional
)

# Link
target_link_libraries(cad-gui-vulkan PRIVATE glfw Vulkan::Vulkan)
