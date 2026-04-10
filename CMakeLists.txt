cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(Wdock)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
pkg_check_modules(Cairo REQUIRED IMPORTED_TARGET GLOBAL cairomm-1.16)

# TODO:
# wayland-scanner private-code wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1.c
# wayland-scanner client-header wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1.h

# wayland-scanner private-code xdg-shell.xml xdg-shell.c
# wayland-scanner client-header xdg-shell.xml xdg-shell.h

file(GLOB IMGUI_CORE_SRC imgui/*.cpp)
list(APPEND IMGUI_SOURCES ${IMGUI_CORE_SRC} imgui/misc/cpp/imgui_stdlib.cpp imgui/backends/imgui_impl_opengl3.cpp imgui/backends/imgui_impl_glfw.cpp)

add_executable(wdock main.cpp ${IMGUI_SOURCES} wlr-layer-shell-unstable-v1.c xdg-shell.c)
target_compile_options(wdock PRIVATE -fsanitize=address,undefined -ggdb -Wall -Wextra -pedantic -Wno-c23-extensions)
target_link_options(wdock PRIVATE -fsanitize=address,undefined)
target_include_directories(wdock PRIVATE glad/include/ imgui/ imgui/backends/ imgui/misc/cpp/)
target_link_libraries(wdock PRIVATE glfw OpenGL PkgConfig::Cairo wayland-client wayland-egl EGL)
