include "Dependencies.lua"

workspace "EppoCore"
    architecture "x86_64"
    startproject "EppoApp"

    configurations {
        "Debug",
        "Release"
    }

    flags {
        "MultiProcessorCompile"
    }

    OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "EppoCore/Vendor/glad"
        include "EppoCore/Vendor/glfw"
        include "EppoCore/Vendor/imgui"
        include "EppoCore/Vendor/spdlog"
    
    group "Core"
        include "EppoCore"
        include "EppoApp"
    group ""
