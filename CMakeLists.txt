cmake_minimum_required(VERSION 4.2.3)

project(Raven VERSION 0.0.0 LANGUAGES CXX DESCRIPTION "Modern C++ Game Engine")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Build"
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Build/modules")

include(ProjectOptions)
raven_setup_options()

include(Dependencies)

add_subdirectory(Engine/Source/Raven)
