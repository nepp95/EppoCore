cmake_minimum_required(VERSION 3.14)

# Project
project(EppoCore VERSION 1.0 LANGUAGES CXX)

cmake_policy(SET CMP0169 OLD)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(EPPO_BUILD_HEADLESS "Build the app in headless mode" ON)
option(EPPO_BUILD_GUI "Build the app in gui mode" ON)

# Dependencies
include(Misc/Dependencies.cmake)

# Files
file(GLOB_RECURSE SOURCES_GUI "EppoCore/Source/*.cpp")
file(GLOB_RECURSE SOURCES_HEADLESS "EppoCore/Source/*.cpp")

set(PCH_HEADER "EppoCore/Source/pch.h")
set(PCH_SOURCE "EppoCore/Source/pch.cpp")

set(INCLUDE_DIRS_GUI
    "EppoCore/Source"
    "EppoCore/Vendor/glad/include"
)

set(INCLUDE_DIRS_HEADLESS
    "EppoCore/Source"
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
    target_compile_definitions(EppoCore_GUI PUBLIC EP_PLATFORM_LINUX EP_GUI)
    target_compile_definitions(EppoCore_Headless PUBLIC EP_PLATFORM_LINUX EP_HEADLESS)
elseif (CMAKE_HOST_WIN32)
    target_compile_definitions(EppoCore_GUI PUBLIC EP_PLATFORM_WINDOWS EP_GUI)
    target_compile_definitions(EppoCore_Headless PUBLIC EP_PLATFORM_WINDOWS EP_HEADLESS)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(EppoCore_GUI PUBLIC EP_DEBUG)
    target_compile_definitions(EppoCore_Headless PUBLIC EP_DEBUG)
else ()
    target_compile_definitions(EppoCore_GUI PUBLIC EP_RELEASE)
    target_compile_definitions(EppoCore_Headless PUBLIC EP_RELEASE)
endif ()

# Packages
add_subdirectory("EppoCore/Vendor/glad")

target_link_libraries(EppoCore_GUI PUBLIC glad glfw glm imgui spdlog)
target_link_libraries(EppoCore_Headless PUBLIC glm spdlog)
target_include_directories(EppoCore_GUI PUBLIC ${INCLUDE_DIRS_GUI})
target_include_directories(EppoCore_Headless PUBLIC ${INCLUDE_DIRS_HEADLESS})
target_precompile_headers(EppoCore_GUI PUBLIC ${PCH_HEADER})
target_precompile_headers(EppoCore_Headless PUBLIC ${PCH_HEADER})
