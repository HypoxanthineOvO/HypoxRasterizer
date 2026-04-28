#include "TestUtils.hpp"

#include "camera.hpp"
#include "configs.hpp"
#include "geometry.hpp"
#include "image.hpp"
#include "object.hpp"

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

std::string projectPath(const std::string& relative) {
    return (findProjectRoot() / relative).string();
}

void testTriangleInterpolation2D() {
    Triangle tri(Vertex(0, 0, 0), Vertex(1, 0, 0), Vertex(0, 1, 0));
    Vec3f p(0.25f, 0.25f, 0.0f);

    test::require(tri.isInsidefor2D(p), "point should be inside the triangle");
    Vec3f weights = tri.getInterpolationWeightsfor2D(p);
    test::requireNear(weights.x() + weights.y() + weights.z(), 1.0f, 1e-5f, "weight sum");

    Vec3f reconstructed =
        weights.x() * tri.getVertex(0).position +
        weights.y() * tri.getVertex(1).position +
        weights.z() * tri.getVertex(2).position;
    test::requireNear(reconstructed.x(), p.x(), 1e-5f, "reconstructed x");
    test::requireNear(reconstructed.y(), p.y(), 1e-5f, "reconstructed y");
}

void testCameraDefaultViewBasis() {
    Camera camera(Vec3f(0, 0, 0), Vec3f(0, 0, -1));
    Mat4f view = camera.getViewMatrix();
    Mat4f expected = Mat4f::Identity();

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            test::requireNear(view(r, c), expected(r, c), 1e-5f, "default view matrix");
        }
    }

    camera.setResolution(Vec2i(64, 32));
    test::require(camera.getWidth() == 64, "camera width should follow Vec2i.x");
    test::require(camera.getHeight() == 32, "camera height should follow Vec2i.y");
}

void testImageWriteCreatesPng() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs";
    std::filesystem::create_directories(output_dir);
    const std::string path = (output_dir / "baseline_color.png").string();
    std::filesystem::remove(path);

    Vec2i resolution(2, 2);
    std::vector<Vec3f> pixels = {
        Vec3f(1, 0, 0), Vec3f(0, 1, 0),
        Vec3f(0, 0, 1), Vec3f(1, 1, 1),
    };

    writeImageToFile(pixels, resolution, path);
    test::require(std::filesystem::exists(path), "PNG output should exist");
    test::require(std::filesystem::file_size(path) > 0, "PNG output should not be empty");
}

void testConfigParsesSmokeScene() {
    Config config(projectPath("configs/SmokeTest.json"));
    test::require(config.camera_config.resolution == Vec2i(32, 32), "smoke resolution should be 32x32");
    test::require(config.materials_config.size() == 1, "smoke scene should contain one material");
    test::require(config.objects_config.size() == 1, "smoke scene should contain one object");
    test::require(config.lights_config.size() == 1, "smoke scene should contain one light");
}

void testInvalidConfigThrows() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs";
    std::filesystem::create_directories(output_dir);
    const std::string path = (output_dir / "invalid_config.json").string();
    {
        std::ofstream out(path);
        out << "{ invalid json";
    }

    test::requireThrows([&]() { Config config(path); }, "invalid JSON config");
}

void testObjectLoadsTriangleAsset() {
    Object object(projectPath("assets/Triangle/scene1.obj"));
    object.localToWorld(Mat4f::Identity());
    test::require(object.getTriangles().size() == 2, "scene1.obj should contain two triangles");
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("triangle interpolation in 2D", testTriangleInterpolation2D);
    runner.run("camera default view basis", testCameraDefaultViewBasis);
    runner.run("image writer creates PNG", testImageWriteCreatesPng);
    runner.run("config parser loads smoke scene", testConfigParsesSmokeScene);
    runner.run("invalid config throws", testInvalidConfigThrows);
    runner.run("object loader reads triangle asset", testObjectLoadsTriangleAsset);
    return runner.finish();
}
