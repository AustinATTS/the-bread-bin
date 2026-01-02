cmake_minimum_required(VERSION 3.20)
project(TheBreadBin VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_COMPILER_LAUNCHER ccache)

# Find external dependencies
find_package(yaml-cpp REQUIRED)

# Add modules
add_subdirectory(core)
add_subdirectory(apps/gui-editor)
add_subdirectory(apps/tui-editor)
