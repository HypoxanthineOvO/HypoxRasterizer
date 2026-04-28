#include "TestUtils.hpp"

#include "image.hpp"
#include "materials.hpp"
#include "scene_config.hpp"

#include <cmath>
#include <filesystem>
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

void testColorMaterialSampleDefaults() {
    ColorMaterial material(Vec3f(0.2f, 0.4f, 0.6f), 32.0f);
    MaterialSample sample = material.sample(Vec2f(0.3f, 0.7f));

    test::requireNear(sample.base_color.x(), 0.2f, 1e-6f, "base color x");
    test::requireNear(sample.base_color.y(), 0.4f, 1e-6f, "base color y");
    test::requireNear(sample.base_color.z(), 0.6f, 1e-6f, "base color z");
    test::requireNear(sample.shininess, 32.0f, 1e-6f, "shininess");
    test::requireNear(sample.roughness, 0.5f, 1e-6f, "default roughness");
    test::requireNear(sample.metallic, 0.0f, 1e-6f, "default metallic");
}

void testPbrMaterialSampleParameters() {
    PBRMaterial material(Vec3f(0.8f, 0.7f, 0.6f), 0.25f, 0.9f);
    MaterialSample sample = material.sample(Vec2f(0.5f, 0.5f));

    test::requireNear(sample.base_color.x(), 0.8f, 1e-6f, "pbr base color");
    test::requireNear(sample.roughness, 0.25f, 1e-6f, "pbr roughness");
    test::requireNear(sample.metallic, 0.9f, 1e-6f, "pbr metallic");
}

void testTextureMaterialUsesActualImageSize() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/materials";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path texture_path = output_dir / "tiny_texture.png";

    Vec2i resolution(2, 2);
    std::vector<Vec3f> pixels = {
        Vec3f(1, 0, 0), Vec3f(0, 1, 0),
        Vec3f(0, 0, 1), Vec3f(1, 1, 1),
    };
    writeImageToFile(pixels, resolution, texture_path.string());

    TextureMaterial material(texture_path.string(), 16.0f);
    test::require(material.getTextureWidth() == 2, "texture width should be loaded from file");
    test::require(material.getTextureHeight() == 2, "texture height should be loaded from file");

    MaterialSample sample = material.sample(Vec2f(0.99f, 0.99f));
    test::require(std::isfinite(sample.base_color.x()), "sample color x should be finite");
    test::require(std::isfinite(sample.base_color.y()), "sample color y should be finite");
    test::require(std::isfinite(sample.base_color.z()), "sample color z should be finite");
}

void testSceneConfigParsesMaterialParams() {
    SceneConfig scene = loadSceneConfig((findProjectRoot() / "configs/SmokeTest.v2.json").string());
    test::require(scene.materials.size() == 1, "one material expected");
    test::requireNear(scene.materials[0].roughness, 0.45f, 1e-6f, "scene material roughness");
    test::requireNear(scene.materials[0].metallic, 0.1f, 1e-6f, "scene material metallic");
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("ColorMaterial sample defaults", testColorMaterialSampleDefaults);
    runner.run("PBRMaterial sample parameters", testPbrMaterialSampleParameters);
    runner.run("TextureMaterial uses actual image size", testTextureMaterialUsesActualImageSize);
    runner.run("SceneConfig parses material params", testSceneConfigParsesMaterialParams);
    return runner.finish();
}
