include "../../Dependencies.lua"

include "Vendor/glad"
include "Vendor/glfw"
include "Vendor/imgui"
include "Vendor/spdlog"

project "EppoCore"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "Off"

    targetdir ("%{wks.location}/Bin/" .. OutputDir .. "/%{prj.name}")
    objdir ("%{wks.location}/Bin-Int/" .. OutputDir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "Source/pch.cpp"

    files {
        "Source/**.h",
        "Source/**.cpp",

		"%{IncludeDir.stb}/*.h",
		"%{IncludeDir.stb}/*.cpp"
    }

    includedirs {
        "Source",

        "%{IncludeDir.glad}",
        "%{IncludeDir.glfw}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.imgui}",
		"%{IncludeDir.spdlog}",
        "%{IncludeDir.stb}"
    }

	links {
		"glad",
		"glfw",
        "imgui",
		"spdlog",
	}

    filter "system:windows"
        defines {
            "EPPO_WINDOWS"
        }

        links {
		    "opengl32.lib"
        }
    
    filter "system:linux"
        defines {
            "EPPO_LINUX"
        }

        links {
            "libGL"
        }

    filter "configurations:Debug"
        defines "EPPO_DEBUG"
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines "EPPO_RELEASE"
        runtime "Release"
        optimize "On"
