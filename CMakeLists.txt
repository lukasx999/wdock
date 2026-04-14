cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

project(Wdock)

find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
pkg_check_modules(Cairo REQUIRED IMPORTED_TARGET GLOBAL cairomm-1.16)


file(GLOB IMGUI_CORE_SRC deps/imgui/*.cpp)
list(APPEND IMGUI_SOURCES ${IMGUI_CORE_SRC}
deps/imgui/misc/cpp/imgui_stdlib.cpp
deps/imgui/backends/imgui_impl_opengl3.cpp
src/imgui_impl_wayland.cpp)

add_executable(wdock ${IMGUI_SOURCES})

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

add_subdirectory(src)

target_compile_options(wdock PRIVATE -fsanitize=address,undefined -ggdb -Wall -Wextra -pedantic -Wno-c23-extensions)
target_link_options(wdock PRIVATE -fsanitize=address,undefined)
target_include_directories(wdock PRIVATE deps/glad/include/ deps/imgui/ deps/imgui/backends/ deps/imgui/misc/cpp/ deps/)
target_include_directories(wdock PRIVATE ${PROJECT_BINARY_DIR})
target_link_libraries(wdock PRIVATE PkgConfig::Cairo wayland-client wayland-egl OpenGL EGL)
