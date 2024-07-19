cmake_minimum_required(VERSION 3.14)

# Project
project(EppoCore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Files
file(GLOB_RECURSE SOURCES "EppoCore/Source/*.cpp")

set(INCLUDE_DIRS
    "EppoCore/Source"
    "EppoCore/Vendor/glad/include"
    "EppoCore/Vendor/glfw/include"
    "EppoCore/Vendor/glm"
    "EppoCore/Vendor/imgui"
    "EppoCore/Vendor/spdlog/include"
)

set(PCH_HEADER "EppoCore/Source/pch.h")
set(PCH_SOURCE "EppoCore/Source/pch.cpp")

# Packages
add_subdirectory("EppoCore/Vendor/glad")
add_subdirectory("EppoCore/Vendor/glfw")
add_subdirectory("EppoCore/Vendor/imgui")

# Create target
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_compile_definitions(${PROJECT_NAME} PRIVATE EPPO_LINUX)
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIRS})
target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH_HEADER})