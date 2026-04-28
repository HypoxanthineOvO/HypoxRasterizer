#include "TestUtils.hpp"

#include "cli.hpp"
#include "output_manager.hpp"
#include "scene_config.hpp"

#include <algorithm>
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

std::string projectPath(const std::string& relative) {
    return (findProjectRoot() / relative).string();
}

bool contains(const std::vector<std::string>& values, const std::string& expected) {
    return std::find(values.begin(), values.end(), expected) != values.end();
}

void testCliParsesRequiredOptions() {
    std::vector<const char*> argv = {
        "HypoxRasterizer",
        "--config", "configs/SmokeTest.v2.json",
        "--output-root", "outputs",
        "--buffers", "color,gbuffer,shadow",
        "--resolution", "64x32",
        "--debug"
    };

    CliOptions options = parseCliOptions(static_cast<int>(argv.size()), argv.data());

    test::require(options.config_path == "configs/SmokeTest.v2.json", "config path should parse");
    test::require(options.output_root == "outputs", "output root should parse");
    test::require(options.debug, "debug flag should parse");
    test::require(options.has_resolution_override, "resolution override should be present");
    test::require(options.resolution_override == Vec2i(64, 32), "resolution override should parse as width x height");
    test::require(contains(options.buffers, "color"), "color buffer should parse");
    test::require(contains(options.buffers, "gbuffer"), "gbuffer buffer should parse");
    test::require(contains(options.buffers, "shadow"), "shadow buffer should parse");
}

void testCliRejectsInvalidResolution() {
    std::vector<const char*> argv = {
        "HypoxRasterizer",
        "--config", "configs/SmokeTest.v2.json",
        "--resolution", "64"
    };

    test::requireThrows([&]() {
        parseCliOptions(static_cast<int>(argv.size()), argv.data());
    }, "invalid resolution");
}

void testSceneConfigLoadsV2Schema() {
    SceneConfig scene = loadSceneConfig(projectPath("configs/SmokeTest.v2.json"));

    test::require(scene.name == "smoke_v2", "scene name should load");
    test::require(scene.camera.resolution == Vec2i(32, 32), "camera resolution should load");
    test::require(scene.materials.size() == 1, "one material expected");
    test::require(scene.objects.size() == 1, "one object expected");
    test::require(scene.lights.size() == 1, "one light expected");
    test::require(scene.lights[0].type == SceneLightType::Point, "point light expected");
    test::require(scene.outputs.root == "outputs", "output root should load");
    test::require(contains(scene.outputs.buffers, "color"), "color output should load");
    test::require(contains(scene.outputs.buffers, "gbuffer"), "gbuffer output should load");
    test::require(contains(scene.outputs.buffers, "shadow"), "shadow output should load");
}

void testSceneConfigRejectsMissingSceneName() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path path = output_dir / "missing_scene_name.v2.json";
    {
        std::ofstream out(path);
        out << R"({
            "scene": {},
            "camera": {
                "resolution": [32, 32],
                "position": [0, 0, 0],
                "target": [0, 0, -1],
                "fov": 45
            },
            "materials": [],
            "objects": [],
            "lights": [],
            "outputs": {"root": "outputs", "buffers": ["color"]}
        })";
    }

    test::requireThrows([&]() {
        loadSceneConfig(path.string());
    }, "missing scene.name");
}

void testOutputManagerCreatesSceneLayout() {
    const std::filesystem::path root = findProjectRoot() / "build/test_outputs/m2_outputs";
    std::filesystem::remove_all(root);

    OutputManager outputs(root.string(), "scene_cli");
    outputs.prepare({"color", "gbuffer", "shadow"});

    test::require(std::filesystem::exists(outputs.sceneRoot()), "scene output root should exist");
    test::require(std::filesystem::exists(outputs.gbufferDir()), "gbuffer directory should exist");
    test::require(std::filesystem::exists(outputs.shadowDir()), "shadow directory should exist");
    test::require(outputs.colorPath().filename() == "color.png", "color path should be color.png");
}

void testOutputManagerHonorsBufferSelection() {
    const std::filesystem::path project_root = findProjectRoot();
    const std::filesystem::path legacy_dir = project_root / "build/test_outputs/m2_legacy_outputs";
    const std::filesystem::path output_root = project_root / "build/test_outputs/m2_selected_outputs";
    std::filesystem::remove_all(legacy_dir);
    std::filesystem::remove_all(output_root);
    std::filesystem::create_directories(legacy_dir);
    {
        std::ofstream(legacy_dir / "color.png") << "color";
        std::ofstream(legacy_dir / "depth.png") << "depth";
        std::ofstream(legacy_dir / "normal.png") << "normal";
        std::ofstream(legacy_dir / "PointlightShadowMap.png_0.png") << "shadow";
    }

    OutputManager outputs(output_root.string(), "selected");
    outputs.prepare({"color"});
    outputs.publishLegacyRendererOutputs({"color"}, legacy_dir);

    test::require(std::filesystem::exists(outputs.colorPath()), "selected color output should exist");
    test::require(!std::filesystem::exists(outputs.gbufferDir() / "depth.png"), "gbuffer should not be published when not selected");
    test::require(!std::filesystem::exists(outputs.shadowDir() / "PointlightShadowMap.png_0.png"), "shadow should not be published when not selected");
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("CLI parses required options", testCliParsesRequiredOptions);
    runner.run("CLI rejects invalid resolution", testCliRejectsInvalidResolution);
    runner.run("SceneConfig loads v2 schema", testSceneConfigLoadsV2Schema);
    runner.run("SceneConfig rejects missing scene name", testSceneConfigRejectsMissingSceneName);
    runner.run("OutputManager creates scene layout", testOutputManagerCreatesSceneLayout);
    runner.run("OutputManager honors buffer selection", testOutputManagerHonorsBufferSelection);
    return runner.finish();
}
