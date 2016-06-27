--[[
MicroBuild
Copyright (C) 2016 TwinDrills

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
--]]
			
-- ---------------------------------------------------------------
--		Workspace configuration
-- ---------------------------------------------------------------	
workspace "MicroBuild"
	configurations 
	{ 
		"Debug", 
		"Release", 
		"Shipping" 
	}	
	platforms 
	{ 
		"x64", 
		"x86" 
	}	
	startproject("MicroBuild")
	location(MBRoot .. "/Build/Bootstrap/ProjectFiles")
		
-- ---------------------------------------------------------------
--		Global build configuration
-- ---------------------------------------------------------------		
filter "configurations:Debug"
	defines("MB_DEBUG_BUILD")
	flags 
	{ 
		"FatalCompileWarnings",	
		"MultiProcessorCompile",
		"ShadowedVariables",
		"Symbols",
		"StaticRuntime",
		"C++11"
	}
	rtti "On"
	optimize "Off"	
	strictaliasing "Level3"
	runtime "Debug"
	
filter "configurations:Release"	
	defines("MB_RELEASE_BUILD")
	flags 
	{ 
		"FatalCompileWarnings",	
		"MultiProcessorCompile",
		"ShadowedVariables",
		"Symbols",
		"StaticRuntime",
		"C++11"
	}
	rtti "On"
	optimize "Debug"
	strictaliasing "Level3"
	runtime "Release"
	
filter "configurations:Shipping"
	defines("MB_SHIPPING_BUILD")
	flags 
	{ 
		"FatalCompileWarnings",	
		"MultiProcessorCompile",
		"LinkTimeOptimization",
		"NoRuntimeChecks",
		"ShadowedVariables",
		"Symbols",
		"StaticRuntime",
		"C++11"
	}
	rtti "On"
	optimize "Full"
	strictaliasing "Level3"
	runtime "Release"				
				
-- ---------------------------------------------------------------
--		Toolset Defines
-- ---------------------------------------------------------------		
filter "system:windows"
	toolset "v140"
filter{}
	
-- ---------------------------------------------------------------
--		Platform Defines
-- ---------------------------------------------------------------			
includedirs({ MBRoot .. "/Source" })
