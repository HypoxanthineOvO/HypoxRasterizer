add_languages("c++17")

add_rules("mode.release")
local depends = {
    "eigen", "stb", "nlohmann_json", "openmp"
}
add_requires(depends)

target("Utils")
    add_packages(depends, {public = true})
    set_kind("static")
    add_includedirs("Utils", {public = true})
    -- add_files("Utils/*.cpp")

target("Rasterizer")
    add_deps("Utils")
    set_kind("static")
    add_includedirs("Modules/Camera", {public = true})
    -- TODO: Add all thr directories
    add_includedirs("Rasterizer", {public = true})

target("HypoxRasterizer")
    add_deps("Rasterizer")
    set_kind("binary")
    set_targetdir(".")
    add_files("main.cpp")