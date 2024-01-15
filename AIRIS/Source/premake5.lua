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
		"Core/**.hpp",
		"Core/**.cpp",
		"Utils/**.h",
		"Utils/**.hpp",
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
			"GLM_FORCE_LEFT_HANDED",
			"GLM_FORCE_DEPTH_ZERO_TO_ONE"
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