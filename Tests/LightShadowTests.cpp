#include "TestUtils.hpp"

#include "configs.hpp"
#include "light.hpp"
#include "output_manager.hpp"
#include "scene_config.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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

void testPointLightCubeFaceLabels() {
    PointLight light(Vec3f(0, 0, 0), Vec3f(1, 1, 1));
    std::vector<std::string> labels = light.getCubeShadowFaceLabels();

    test::require(labels.size() == 6, "point light should expose six cube shadow faces");
    test::require(labels[0] == "+X", "face 0 should be +X");
    test::require(labels[1] == "-X", "face 1 should be -X");
    test::require(labels[2] == "+Y", "face 2 should be +Y");
    test::require(labels[3] == "-Y", "face 3 should be -Y");
    test::require(labels[4] == "+Z", "face 4 should be +Z");
    test::require(labels[5] == "-Z", "face 5 should be -Z");
}

void testDirectionalLightNormalizesDirection() {
    DirectionalLight light(Vec3f(0, -2, 0), Vec3f(1, 1, 1));
    Vec3f direction = light.getDirection();

    test::requireNear(direction.x(), 0.0f, 1e-6f, "direction x");
    test::requireNear(direction.y(), -1.0f, 1e-6f, "direction y");
    test::requireNear(direction.z(), 0.0f, 1e-6f, "direction z");
}

void testSceneConfigParsesDirectionalLight() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/lights";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path config_path = output_dir / "directional.v2.json";
    {
        std::ofstream out(config_path);
        out << R"({
            "scene": {"name": "directional_test"},
            "camera": {
                "resolution": [16, 16],
                "position": [0, 0, 1],
                "target": [0, 0, 0],
                "fov": 45
            },
            "materials": [],
            "objects": [],
            "lights": [{
                "name": "sun",
                "type": "directional",
                "direction": [0, -2, 0],
                "intensity": [1, 1, 1],
                "shadow": {"enabled": true}
            }],
            "outputs": {"root": "outputs", "buffers": ["color", "shadow"]}
        })";
    }

    SceneConfig scene = loadSceneConfig(config_path.string());
    test::require(scene.lights.size() == 1, "one light expected");
    test::require(scene.lights[0].type == SceneLightType::Directional, "directional light expected");
    test::requireNear(scene.lights[0].direction.y(), -2.0f, 1e-6f, "directional light direction should load");
    test::require(scene.lights[0].shadow_enabled, "directional shadow flag should load");
}

void testOutputManagerPublishesDirectionalAndPointShadows() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/light_publish";
    const std::filesystem::path working_dir = output_dir / "work";
    std::filesystem::remove_all(output_dir);
    std::filesystem::create_directories(working_dir);

    {
        std::ofstream out(working_dir / "PointlightShadowMap_0_face_pos_x.png");
        out << "point-shadow";
    }
    {
        std::ofstream out(working_dir / "DirectionalLightShadowMap_1.png");
        out << "directional-shadow";
    }

    OutputManager outputs((output_dir / "out").string(), "scene");
    outputs.prepare({"shadow"});
    outputs.publishLegacyRendererOutputs({"shadow"}, working_dir);

    test::require(
        std::filesystem::exists(output_dir / "out/scene/shadow/PointlightShadowMap_0_face_pos_x.png"),
        "point cube shadow face should publish"
    );
    test::require(
        std::filesystem::exists(output_dir / "out/scene/shadow/DirectionalLightShadowMap_1.png"),
        "directional shadow map should publish"
    );
}

void testLegacyConfigParsesShadowEnabledFlag() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/lights";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path config_path = output_dir / "legacy_shadow_disabled.json";
    {
        std::ofstream out(config_path);
        out << R"({
            "Camera": {
                "Resolution": [8, 8],
                "Position": [0, 0, 1],
                "Target": [0, 0, 0],
                "FocalLength": 1,
                "Fov": 45
            },
            "Lights": [{
                "Type": "PointLight",
                "Position": [0, 1, 0],
                "Intensity": [1, 1, 1],
                "ShadowEnabled": false
            }],
            "Materials": [],
            "Objects": []
        })";
    }

    Config config(config_path.string());
    test::require(config.lights_config.size() == 1, "one legacy light expected");
    test::require(!config.lights_config[0].shadow_enabled, "legacy shadow flag should load false");
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("PointLight cube face labels", testPointLightCubeFaceLabels);
    runner.run("DirectionalLight normalizes direction", testDirectionalLightNormalizesDirection);
    runner.run("SceneConfig parses directional light", testSceneConfigParsesDirectionalLight);
    runner.run("OutputManager publishes directional and point shadows", testOutputManagerPublishesDirectionalAndPointShadows);
    runner.run("Legacy config parses shadow enabled flag", testLegacyConfigParsesShadowEnabledFlag);
    return runner.finish();
}
