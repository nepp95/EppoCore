cmake_minimum_required(VERSION 3.14)

# Project
project(EppoCore VERSION 1.0 LANGUAGES CXX)

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
    "EppoCore/Vendor/imgui"
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

# ImGui's Vulkan backend, vendored (matching vcpkg's imgui version) so it builds against the Vulkan SDK
set(IMGUI_VULKAN_SOURCE "EppoCore/Vendor/imgui/imgui_impl_vulkan.cpp")
set_source_files_properties(${IMGUI_VULKAN_SOURCE} PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

# Create target
add_library(EppoCore_GUI STATIC ${SOURCES_GUI} ${IMGUI_VULKAN_SOURCE})
add_library(EppoCore_Headless STATIC ${SOURCES_HEADLESS})

if (CMAKE_HOST_LINUX)
    target_compile_definitions(EppoCore_GUI PUBLIC EP_PLATFORM_LINUX EP_GUI)
    target_compile_definitions(EppoCore_Headless PUBLIC EP_PLATFORM_LINUX EP_HEADLESS)
elseif (CMAKE_HOST_WIN32)
    target_compile_definitions(EppoCore_GUI PUBLIC EP_PLATFORM_WINDOWS EP_GUI)
    target_compile_definitions(EppoCore_Headless PUBLIC EP_PLATFORM_WINDOWS EP_HEADLESS)
endif ()

# Generator expression so multi-config generators (Visual Studio) get the right define per configuration
target_compile_definitions(EppoCore_GUI PUBLIC $<IF:$<CONFIG:Debug>,EP_DEBUG,EP_RELEASE>)
target_compile_definitions(EppoCore_Headless PUBLIC $<IF:$<CONFIG:Debug>,EP_DEBUG,EP_RELEASE>)

# Packages
target_link_libraries(EppoCore_GUI
    PUBLIC glfw glm::glm imgui::imgui spdlog::spdlog Tracy::TracyClient Vulkan::Vulkan
    PRIVATE Vulkan::shaderc_combined
)
target_link_libraries(EppoCore_Headless
    PUBLIC glm::glm spdlog::spdlog Tracy::TracyClient Vulkan::Vulkan
    PRIVATE Vulkan::shaderc_combined
)

if (CMAKE_HOST_LINUX)
    target_link_directories(EppoCore_GUI PUBLIC "$ENV{VULKAN_SDK}/lib")
    target_link_directories(EppoCore_Headless PUBLIC "$ENV{VULKAN_SDK}/lib")
    target_link_libraries(EppoCore_GUI PUBLIC spirv-cross-core)
    target_link_libraries(EppoCore_Headless PUBLIC spirv-cross-core)
elseif (CMAKE_HOST_WIN32)
    # Only the Windows SDK ships separate debug builds ('d' suffix)
    target_link_directories(EppoCore_GUI PUBLIC "$ENV{VULKAN_SDK}/Lib")
    target_link_directories(EppoCore_Headless PUBLIC "$ENV{VULKAN_SDK}/Lib")
    target_link_libraries(EppoCore_GUI PUBLIC $<IF:$<CONFIG:Debug>,spirv-cross-cored,spirv-cross-core>)
    target_link_libraries(EppoCore_Headless PUBLIC $<IF:$<CONFIG:Debug>,spirv-cross-cored,spirv-cross-core>)
endif ()

target_include_directories(EppoCore_GUI PUBLIC ${INCLUDE_DIRS_GUI})
target_include_directories(EppoCore_Headless PUBLIC ${INCLUDE_DIRS_HEADLESS})
target_precompile_headers(EppoCore_GUI PUBLIC ${PCH_HEADER})
target_precompile_headers(EppoCore_Headless PUBLIC ${PCH_HEADER})
