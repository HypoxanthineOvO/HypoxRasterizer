add_languages("c++17")

add_rules("plugin.compile_commands.autoupdate", {outputdir = ".vscode"})

add_rules("mode.release")
local depends = {
    "eigen", "stb", "nlohmann_json", "openmp", "tinyobjloader"
}
add_requires(depends)

target("Utils")
    add_packages(depends, {public = true})
    set_kind("static")
    add_includedirs("Utils", {public = true})
    add_includedirs("Utils/Image", {public = true})
    add_includedirs("Utils/Configs", {public = true})
    add_files("Utils/Image/*.cpp")
    add_files("Utils/Configs/*.cpp")

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
    -- TODO: Add all thr directories
    -- Rasterizer
    add_includedirs("Rasterizer", {public = true})
    add_files("Rasterizer/*.cpp")

target("HypoxRasterizer")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir(".")
    add_files("main.cpp")

---------- Testcases ----------
-- target("CameraTest")
--     add_deps("Utils")
--     set_kind("binary")
--     add_includedirs("Modules/Camera", {public = true})
--     add_files("Modules/Camera/*.cpp")
--     add_files("Tests/CameraTest.cpp")
--     add_packages(depends, {public = true})
--     set_targetdir(".")

-- target("ImageTest")
--     add_deps("Utils")
--     set_kind("binary")
--     add_includedirs("Utils/Image", {public = true})
--     add_files("Utils/Image/*.cpp")
--     add_files("Tests/ImageTest.cpp")
--     add_packages(depends, {public = true})
--     set_targetdir(".")

-- target("TriangleTest")
--     add_deps("Utils")
--     set_kind("binary")
--     add_includedirs("Modules/Object/")
--     add_files("Modules/Object/geometry.cpp")
--     add_files("Tests/TriangleTest.cpp")
--     add_packages(depends, {public = true})
--     set_targetdir(".")

-- target("ObjectTest")
--     add_deps("Utils")
--     set_kind("binary")
--     add_includedirs("Modules/Object/")
--     add_files("Modules/Object/geometry.cpp")
--     add_files("Modules/Object/object.cpp")
--     add_files("Tests/ObjectTest.cpp")
--     add_packages(depends, {public = true})
--     set_targetdir(".")

--     target("TriangleTestpy")
--     add_deps("Utils")
--     set_kind("binary")
--     add_includedirs("Modules/Object/")
--     add_files("Modules/Object/geometry.cpp")
--     add_files("Tests/TriangleTestByPython/TriangleTest.cpp")
--     add_packages(depends, {public = true})
--     set_targetdir(".")