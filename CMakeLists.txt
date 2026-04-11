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


file(GLOB IMGUI_CORE_SRC imgui/*.cpp)
list(APPEND IMGUI_SOURCES ${IMGUI_CORE_SRC}
imgui/misc/cpp/imgui_stdlib.cpp
imgui/backends/imgui_impl_opengl3.cpp
imgui/backends/imgui_impl_glfw.cpp
imgui/backends/imgui_impl_null.cpp)

add_executable(wdock main.cpp ${IMGUI_SOURCES} wayland.cpp)




add_custom_command(
  OUTPUT xdg-shell.c
  COMMAND wayland-scanner private-code ${PROJECT_SOURCE_DIR}/protocols/xdg-shell.xml xdg-shell.c
  DEPENDS protocols
)
target_sources(wdock PRIVATE xdg-shell.c)

add_custom_command(
  OUTPUT xdg-shell.h
  COMMAND wayland-scanner client-header ${PROJECT_SOURCE_DIR}/protocols/xdg-shell.xml xdg-shell.h
  DEPENDS protocols
)
add_custom_target(xdg-shell DEPENDS xdg-shell.h)
add_dependencies(wdock xdg-shell)

add_custom_command(
  OUTPUT wlr-layer-shell-unstable-v1.h
  COMMAND wayland-scanner client-header ${PROJECT_SOURCE_DIR}/protocols/wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1.h
  DEPENDS protocols
)
add_custom_target(wlr-layer-shell DEPENDS wlr-layer-shell-unstable-v1.h)
add_dependencies(wdock wlr-layer-shell)

add_custom_command(
  OUTPUT wlr-layer-shell-unstable-v1.c
  COMMAND wayland-scanner private-code ${PROJECT_SOURCE_DIR}/protocols/wlr-layer-shell-unstable-v1.xml wlr-layer-shell-unstable-v1.c
  DEPENDS protocols
)
target_sources(wdock PRIVATE wlr-layer-shell-unstable-v1.c)


target_compile_options(wdock PRIVATE -fsanitize=address,undefined -ggdb -Wall -Wextra -pedantic -Wno-c23-extensions)
target_link_options(wdock PRIVATE -fsanitize=address,undefined)
target_include_directories(wdock PRIVATE glad/include/ imgui/ imgui/backends/ imgui/misc/cpp/)
target_include_directories(wdock PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(wdock PRIVATE glfw OpenGL PkgConfig::Cairo wayland-client wayland-egl EGL)
