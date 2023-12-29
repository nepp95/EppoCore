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
		"%{IncludeDir.spdlog}",
        "%{IncludeDir.stb}"
    }

	links {
		"Glad",
		"Glfw",
		"Spdlog",
		"opengl32.lib"
	}

    filter "configurations:Debug"
        defines "EPPO_DEBUG"
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines "EPPO_RELEASE"
        runtime "Release"
        optimize "On"
