#include "TestUtils.hpp"

#include "float_buffer_writer.hpp"
#include "gbuffer.hpp"

#include <cmath>
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

void testGBufferStoresFullSample() {
    GBuffer gbuffer(Vec2i(2, 2));
    GBufferSample sample;
    sample.world_position = Vec3f(1, 2, 3);
    sample.view_position = Vec3f(4, 5, 6);
    sample.linear_depth = 7.0f;
    sample.world_normal = Vec3f(0, 1, 0);
    sample.view_normal = Vec3f(0, 0, 1);
    sample.albedo = Vec3f(0.2f, 0.3f, 0.4f);
    sample.material_id = 9;
    sample.roughness = 0.6f;
    sample.metallic = 0.7f;
    sample.shadow_visibility = 0.8f;

    gbuffer.writePixel(1, 0, sample);
    GBufferSample loaded = gbuffer.sample(1, 0);

    test::requireNear(loaded.world_position.x(), 1.0f, 1e-6f, "world position x");
    test::requireNear(loaded.view_position.z(), 6.0f, 1e-6f, "view position z");
    test::requireNear(loaded.linear_depth, 7.0f, 1e-6f, "linear depth");
    test::requireNear(loaded.albedo.y(), 0.3f, 1e-6f, "albedo y");
    test::require(loaded.material_id == 9, "material id");
    test::requireNear(loaded.roughness, 0.6f, 1e-6f, "roughness");
    test::requireNear(loaded.metallic, 0.7f, 1e-6f, "metallic");
    test::requireNear(loaded.shadow_visibility, 0.8f, 1e-6f, "shadow visibility");
}

void testGBufferMetadataListsRequiredChannels() {
    std::vector<BufferChannelInfo> metadata = GBuffer::channelMetadata();
    std::vector<std::string> names;
    for (const auto& channel : metadata) {
        names.push_back(channel.name);
    }

    for (const std::string& required : {
        "world_position", "view_position", "linear_depth", "world_normal",
        "view_normal", "albedo", "material_id", "roughness", "metallic",
        "shadow_visibility"
    }) {
        test::require(std::find(names.begin(), names.end(), required) != names.end(), "missing channel: " + required);
    }
}

void testUniformDepthPreviewIsFinite() {
    GBuffer gbuffer(Vec2i(2, 2));
    GBufferSample sample;
    sample.linear_depth = 5.0f;
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 2; ++x) {
            gbuffer.writePixel(x, y, sample);
        }
    }

    std::vector<float> preview = gbuffer.normalizedLinearDepthPreview();
    test::require(preview.size() == 4, "preview size");
    for (float value : preview) {
        test::require(std::isfinite(value), "depth preview should be finite");
        test::requireNear(value, 0.0f, 1e-6f, "uniform depth preview value");
    }
}

void testFloatBufferWriterHeaderAndSize() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/gbuffer";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path output_file = output_dir / "sample.f32";
    std::filesystem::remove(output_file);

    FloatBuffer buffer;
    buffer.width = 2;
    buffer.height = 1;
    buffer.channels = 3;
    buffer.name = "sample";
    buffer.data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f};
    writeFloatBuffer(output_file, buffer);

    test::require(std::filesystem::exists(output_file), "float buffer output should exist");
    std::ifstream input(output_file, std::ios::binary);
    std::string magic;
    input >> magic;
    test::require(magic == "HYPORASTER_F32_V1", "float buffer magic header");
    test::require(std::filesystem::file_size(output_file) > buffer.data.size() * sizeof(float), "float buffer should include header and payload");
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("GBuffer stores full sample", testGBufferStoresFullSample);
    runner.run("GBuffer metadata lists required channels", testGBufferMetadataListsRequiredChannels);
    runner.run("uniform depth preview is finite", testUniformDepthPreviewIsFinite);
    runner.run("FloatBufferWriter writes header and payload", testFloatBufferWriterHeaderAndSize);
    return runner.finish();
}
