add_languages("c++17")

add_rules("mode.release")
local depends = {
    "eigen", "stb", "nlohmann_json", "openmp"
}
add_requires(depends)
