cmake_minimum_required(VERSION 3.14)

# Project
project(EppoCore VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(EPPO_BUILD_HEADLESS "Build the app in headless mode" ON)
option(EPPO_BUILD_GUI "Build the app in gui mode" ON)

# Files
file(GLOB_RECURSE SOURCES_GUI "EppoCore/Source/*.cpp")
file(GLOB_RECURSE SOURCES_HEADLESS "EppoCore/Source/*.cpp")

set(PCH_HEADER "EppoCore/Source/pch.h")
set(PCH_SOURCE "EppoCore/Source/pch.cpp")

set(INCLUDE_DIRS_GUI
    "EppoCore/Source"
    "EppoCore/Vendor/glad/include"
    "EppoCore/Vendor/glfw/include"
    "EppoCore/Vendor/glm"
    "EppoCore/Vendor/imgui"
    "EppoCore/Vendor/spdlog/include"
)

set(INCLUDE_DIRS_HEADLESS
    "EppoCore/Source"
    "EppoCore/Vendor/glm"
    "EppoCore/Vendor/spdlog/include"
)

file(GLOB_RECURSE GUI_SOURCES "EppoCore/Source/EppoCore/Platform/GUI/*.cpp")
file(GLOB_RECURSE GUI_INCLUDES "EppoCore/Source/EppoCore/Platform/GUI/*.h")
list(REMOVE_ITEM SOURCES_HEADLESS ${GUI_SOURCES})

file(GLOB_RECURSE HEADLESS_SOURCES "EppoCore/Source/EppoCore/Platform/Headless/*.cpp")
file(GLOB_RECURSE HEADLESS_INCLUDES "EppoCore/Source/EppoCore/Platform/Headless/*.h")
list(REMOVE_ITEM SOURCES_GUI ${HEADLESS_SOURCES})

# Create target
add_library(EppoCore_GUI STATIC ${SOURCES_GUI})
add_library(EppoCore_Headless STATIC ${SOURCES_HEADLESS})

if (CMAKE_HOST_LINUX)
    target_compile_definitions(EppoCore_GUI PUBLIC EPPO_LINUX)
    target_compile_definitions(EppoCore_Headless PUBLIC EPPO_LINUX EPPO_HEADLESS)
elseif (CMAKE_HOST_WIN32)
    target_compile_definitions(EppoCore_GUI PUBLIC EPPO_WINDOWS)
    target_compile_definitions(EppoCore_Headless PUBLIC EPPO_WINDOWS EPPO_HEADLESS)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(EppoCore_GUI PUBLIC EPPO_DEBUG)
    target_compile_definitions(EppoCore_Headless PUBLIC EPPO_DEBUG)
else()
    target_compile_definitions(EppoCore_GUI PUBLIC EPPO_RELEASE)
    target_compile_definitions(EppoCore_Headless PUBLIC EPPO_RELEASE)
endif()

# Packages
add_subdirectory("EppoCore/Vendor/glad")
add_subdirectory("EppoCore/Vendor/imgui")
add_subdirectory("EppoCore/Vendor/glfw")

target_link_libraries(EppoCore_GUI PUBLIC glfw glad imgui)
target_include_directories(EppoCore_GUI PUBLIC ${INCLUDE_DIRS_GUI})
target_include_directories(EppoCore_Headless PUBLIC ${INCLUDE_DIRS_HEADLESS})
target_precompile_headers(EppoCore_GUI PUBLIC ${PCH_HEADER})
target_precompile_headers(EppoCore_Headless PUBLIC ${PCH_HEADER})
