project "ComputeRT"
	kind "WindowedApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir (BinDir .. "%{prj.name}")
	objdir (IntDir .. "%{prj.name}")

	files
	{
		"main.cpp",
		"**.h",
		"**.hpp",
		"**.cpp",
	}

	includedirs
	{
		"%{IncludeDir.AIRIS}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",	
	}

	links 
	{
			"AIRIS"
	}

	filter "system:windows"
		systemversion "latest"
		
		defines
		{
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
