
workspace "AIRIS"
	architecture "x64"
	startproject "ComputeRT"

	configurations
	{
		"Debug",
		"Release"
	}

local tempPath = _MAIN_SCRIPT_DIR .. "/"

-- OUTPUT DIRECTORIES
OutDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/"
BinDir = tempPath .. "Build/Bin/" .. OutDir
IntDir = tempPath .. "Build/Intermediates/" .. OutDir

-- EXTERNAL DEPENCENCIES DIR
ExtDep = "ED"

-- INCLUDE DIRECTORIES
IncludeDir = {}
IncludeDir["spdlog"] = "%{ExtDep}/spdlog/include/"
IncludeDir["imgui"] = "%{ExtDep}/imgui/"
IncludeDir["glm"] = "%{ExtDep}/glm/"
IncludeDir["AIRIS"] = "AIRIS/Source/"

print(tempPath)
for k,v in pairs(IncludeDir) do
	IncludeDir[k] = tempPath .. v 
end


group "IMPL"
	include "AIRIS/Source"

group "Apps"
	include "AIRIS/Apps/ComputeRT"




