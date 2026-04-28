add_languages("c++17")

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

add_rules("mode.release", "mode.debug")
local depends = {
    "eigen", "openmp", "tinyobjloader"
}
add_requires(depends)

if is_mode("release") then
    add_cxxflags("-O2", {force = true})
end

if is_mode("debug") then
    add_defines("DEBUG")
    -- add_cxxflags("-O2", {force = true})
end

target("Utils")
    add_packages(depends, {public = true})
    set_kind("static")
    add_includedirs("third_party/include", {public = true})
    add_includedirs("Utils", {public = true})
    add_includedirs("Utils/Image", {public = true})
    add_includedirs("Utils/Configs", {public = true})
    add_includedirs("Utils/Output", {public = true})
    add_includedirs("Utils/Buffer", {public = true})
    add_includedirs("Utils/Validation", {public = true})
    add_files("Utils/Image/*.cpp")
    add_files("Utils/Configs/*.cpp")
    add_files("Utils/Output/*.cpp")
    add_files("Utils/Buffer/*.cpp")
    add_files("Utils/Validation/*.cpp")

target("Rasterizer")
    add_deps("Utils")
    set_kind("static")
    -- Camera
    add_includedirs("Modules/Camera", {public = true})
    add_files("Modules/Camera/*.cpp")
    -- Light
    add_includedirs("Modules/Light", {public = true})
    add_files("Modules/Light/*.cpp")
    -- Object
    add_includedirs("Modules/Object", {public = true})
    add_files("Modules/Object/*.cpp")
    -- Rasterizer
    add_includedirs("Rasterizer", {public = true})
    add_files("Rasterizer/*.cpp")

target("HypoxRasterizer")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir(".")
    add_files("main.cpp")

target("BaselineTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/BaselineTests.cpp")

target("SceneCliOutputTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/SceneCliOutputTests.cpp")

target("GBufferTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/GBufferTests.cpp")

target("MaterialTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/MaterialTests.cpp")

target("LightShadowTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/LightShadowTests.cpp")

target("VPLAreaTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/VPLAreaTests.cpp")

target("IndirectVPLTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/IndirectVPLTests.cpp")

target("ValidationSceneTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/ValidationSceneTests.cpp")

target("ValidationSuiteTests")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir("build/tests")
    add_files("Tests/ValidationSuiteTests.cpp")
