project "AIRIS"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir (BinDir  .. "/%{prj.name}")
	objdir (IntDir .. "/%{prj.name}")

	files
	{
		"*.h",
		"*.hpp",
		"Core/**.h",
		"Core/**.cpp",
		"Utils/**.h",
		"Utils/**.cpp",

	}

	includedirs
	{
		".",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.imgui}",
		"%{IncludeDir.glm}",
	}

	filter "system:windows"
		systemversion "latest"
		
		defines
		{
		}

		links
		{
			"D3D12",
			"dxgi",
			"d3dcompiler",
		}


	filter "configurations:Debug"
		
		defines
		{
		}
		runtime "Debug"
		symbols "on"
		

	filter "configurations:Release"
		
		defines
		{
		}
		runtime "Release"
		optimize "on"