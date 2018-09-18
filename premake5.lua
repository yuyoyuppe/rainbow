workspace "rainbow"
configurations {"Debug", "Release"}
language "C++"

paths = {
  server = "server/",
  build = "build/"
}

newaction {
	trigger = "clean",
	description = "Clean all generated build artifacts",
	onStart = function()
    os.rmdir(paths.build)
  end
}

system "windows"
architecture "x86_64"
cppdialect "C++17"

project "server"
  basedir (paths.build)
  targetdir (paths.build .. "bin")
  objdir (paths.build .. "obj")
  files {paths.server .. "src/**.cpp", paths.server .. "src/**.hpp"}
  includedirs {paths.server .. "src/pch/"}
  pchheader "pch.hpp"
  pchsource (paths.server .. "src/pch/pch.cpp")
  kind "ConsoleApp"
  filter "configurations:Debug"
    symbols "On"
    runtime "Debug"
    defines { "DEBUG" }
    targetsuffix "d"
  filter "configurations:Release"
    symbols "Off"
    defines { "NDEBUG" }
    runtime "Release"
    optimize "On"
