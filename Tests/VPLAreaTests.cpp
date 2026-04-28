#include "TestUtils.hpp"

#include "light.hpp"
#include "scene_config.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path findProjectRoot() {
    std::filesystem::path current = std::filesystem::current_path();
    for (int depth = 0; depth < 8; ++depth) {
        if (std::filesystem::exists(current / "xmake.lua") &&
            std::filesystem::exists(current / "configs") &&
            std::filesystem::exists(current / "assets")) {
            return current;
        }
        if (!current.has_parent_path() || current == current.parent_path()) {
            break;
        }
        current = current.parent_path();
    }
    throw test::Failure("unable to locate project root from current working directory");
}

void testAreaLightUsesConfiguredVPLCount() {
    AreaLight light(
        Vec3f(0, 4, 0),
        Vec3f(8, 4, 2),
        Vec3f(0, -1, 0),
        Vec2f(2, 2),
        64,
        7,
        false
    );

    std::vector<DirectVPL> samples = light.getDirectVPLs();
    test::require(samples.size() == 64, "area light should create configured VPL count");

    Vec3f total_intensity = Vec3f::Zero();
    for (const DirectVPL& sample : samples) {
        total_intensity += sample.intensity;
        test::require(std::abs(sample.position.x()) <= 1.01f, "VPL x should stay inside area bounds");
        test::require(std::abs(sample.position.z()) <= 1.01f, "VPL z should stay inside area bounds");
    }
    test::requireNear(total_intensity.x(), 8.0f, 1e-4f, "total VPL red intensity");
    test::requireNear(total_intensity.y(), 4.0f, 1e-4f, "total VPL green intensity");
    test::requireNear(total_intensity.z(), 2.0f, 1e-4f, "total VPL blue intensity");
}

void testAreaLightStochasticSeedIsReproducible() {
    AreaLight first(Vec3f(0, 4, 0), Vec3f(1, 1, 1), Vec3f(0, -1, 0), Vec2f(2, 2), 16, 11, true);
    AreaLight same_seed(Vec3f(0, 4, 0), Vec3f(1, 1, 1), Vec3f(0, -1, 0), Vec2f(2, 2), 16, 11, true);
    AreaLight different_seed(Vec3f(0, 4, 0), Vec3f(1, 1, 1), Vec3f(0, -1, 0), Vec2f(2, 2), 16, 12, true);

    Vec3f a = first.getDirectVPLs()[0].position;
    Vec3f b = same_seed.getDirectVPLs()[0].position;
    Vec3f c = different_seed.getDirectVPLs()[0].position;

    test::require((a - b).norm() < 1e-6f, "same seed should reproduce VPL positions");
    test::require((a - c).norm() > 1e-4f, "different seed should change stochastic VPL positions");
}

void testSceneConfigParsesAreaVPLFields() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/vpl";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path config_path = output_dir / "area_vpl.v2.json";
    {
        std::ofstream out(config_path);
        out << R"({
            "scene": {"name": "area_vpl_test"},
            "camera": {
                "resolution": [16, 16],
                "position": [0, 1, 4],
                "target": [0, 1, 0],
                "fov": 45
            },
            "materials": [],
            "objects": [],
            "lights": [{
                "name": "panel",
                "type": "area",
                "position": [0, 4, 0],
                "normal": [0, -1, 0],
                "size": [2, 3],
                "intensity": [3, 3, 3],
                "vpl": {"samples": 81, "seed": 23, "stochastic": true},
                "shadow": {"enabled": true}
            }],
            "outputs": {"root": "outputs", "buffers": ["color", "shadow"]}
        })";
    }

    SceneConfig scene = loadSceneConfig(config_path.string());
    test::require(scene.lights.size() == 1, "one light expected");
    test::require(scene.lights[0].type == SceneLightType::Area, "area light expected");
    test::requireNear(scene.lights[0].normal.y(), -1.0f, 1e-6f, "area normal should load");
    test::requireNear(scene.lights[0].size.y(), 3.0f, 1e-6f, "area size should load");
    test::require(scene.lights[0].vpl_sample_count == 81, "VPL sample count should load");
    test::require(scene.lights[0].vpl_seed == 23, "VPL seed should load");
    test::require(scene.lights[0].vpl_stochastic, "VPL stochastic flag should load");
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("AreaLight uses configured VPL count", testAreaLightUsesConfiguredVPLCount);
    runner.run("AreaLight stochastic seed is reproducible", testAreaLightStochasticSeedIsReproducible);
    runner.run("SceneConfig parses area VPL fields", testSceneConfigParsesAreaVPLFields);
    return runner.finish();
}
