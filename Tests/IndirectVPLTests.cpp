#include "TestUtils.hpp"

#include "configs.hpp"
#include "float_buffer_writer.hpp"
#include "indirect_vpl_sampler.hpp"
#include "light.hpp"
#include "rasterizer.hpp"
#include "scene_config.hpp"
#include "vpl_debug.hpp"

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

void testSceneConfigParsesIndirectVPLFields() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/indirect_vpl";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path config_path = output_dir / "indirect_vpl.v2.json";
    {
        std::ofstream out(config_path);
        out << R"({
            "scene": {"name": "indirect_vpl_contract"},
            "camera": {
                "resolution": [16, 16],
                "position": [0, 1, 4],
                "target": [0, 1, 0],
                "fov": 45
            },
            "materials": [],
            "objects": [],
            "lights": [{
                "name": "key",
                "type": "point",
                "position": [0, 4, 0],
                "intensity": [2, 2, 2],
                "indirectVPL": {
                    "enabled": true,
                    "samples": 128,
                    "seed": 31,
                    "bounces": 1,
                    "visibility": false,
                    "clamp": 3.5,
                    "radius": 0.25,
                    "sourceLights": [0]
                }
            }],
            "outputs": {"root": "outputs", "buffers": ["color", "shadow"]}
        })";
    }

    SceneConfig scene = loadSceneConfig(config_path.string());
    test::require(scene.lights.size() == 1, "one light expected");
    test::require(scene.lights[0].indirect_vpl.enabled, "IndirectVPL should parse enabled");
    test::require(scene.lights[0].indirect_vpl.samples == 128, "IndirectVPL samples should parse");
    test::require(scene.lights[0].indirect_vpl.seed == 31, "IndirectVPL seed should parse");
    test::require(scene.lights[0].indirect_vpl.bounces == 1, "IndirectVPL bounces should parse");
    test::require(!scene.lights[0].indirect_vpl.visibility, "IndirectVPL visibility should parse false");
    test::requireNear(scene.lights[0].indirect_vpl.clamp, 3.5f, 1e-6f, "IndirectVPL clamp");
    test::requireNear(scene.lights[0].indirect_vpl.radius, 0.25f, 1e-6f, "IndirectVPL radius");
    test::require(scene.lights[0].indirect_vpl.source_light_indices.size() == 1, "source light list should parse");
}

void testLegacyConfigParsesIndirectVPLFields() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/indirect_vpl";
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path config_path = output_dir / "legacy_indirect_vpl.json";
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
                "IndirectVPL": {
                    "Enabled": true,
                    "Samples": 32,
                    "Seed": 9,
                    "Bounces": 1,
                    "Visibility": true,
                    "Clamp": 2.0,
                    "Radius": 0.5,
                    "SourceLights": [0]
                }
            }],
            "Materials": [],
            "Objects": []
        })";
    }

    Config config(config_path.string());
    test::require(config.lights_config.size() == 1, "one legacy light expected");
    test::require(config.lights_config[0].indirect_vpl.enabled, "legacy IndirectVPL should parse enabled");
    test::require(config.lights_config[0].indirect_vpl.samples == 32, "legacy IndirectVPL samples should parse");
    test::require(config.lights_config[0].indirect_vpl.seed == 9, "legacy IndirectVPL seed should parse");
    test::requireNear(config.lights_config[0].indirect_vpl.radius, 0.5f, 1e-6f, "legacy IndirectVPL radius");
}

void testIndirectVPLSampleFloatRecord() {
    IndirectVPL sample(
        Vec3f(1, 2, 3),
        Vec3f(0, 1, 0),
        Vec3f(0.25f, 0.5f, 0.75f),
        0.4f,
        1,
        2,
        7
    );

    std::vector<float> record = sample.toFloatRecord();
    test::require(record.size() == 13, "IndirectVPL float record should have 13 channels");
    test::requireNear(record[0], 1.0f, 1e-6f, "position x");
    test::requireNear(record[4], 1.0f, 1e-6f, "normal y");
    test::requireNear(record[8], 0.75f, 1e-6f, "flux z");
    test::requireNear(record[9], 0.4f, 1e-6f, "radius");
    test::requireNear(record[10], 1.0f, 1e-6f, "bounce");
    test::requireNear(record[11], 2.0f, 1e-6f, "source light");
    test::requireNear(record[12], 7.0f, 1e-6f, "flags");
}

void testIndirectVPLFloatBufferContract() {
    std::vector<IndirectVPL> samples = {
        IndirectVPL(Vec3f(1, 0, 0), Vec3f(0, 1, 0), Vec3f(1, 1, 1), 0.5f, 1, 0, 0),
        IndirectVPL(Vec3f(0, 1, 0), Vec3f(0, 0, 1), Vec3f(0.5f, 0.5f, 0.5f), 0.25f, 1, 0, 1),
    };
    FloatBuffer buffer = makeIndirectVPLFloatBuffer(0, samples);

    test::require(buffer.name == "light_0_indirect_vpls", "IndirectVPL buffer name");
    test::require(buffer.width == 2, "IndirectVPL buffer width");
    test::require(buffer.height == 1, "IndirectVPL buffer height");
    test::require(buffer.channels == 13, "IndirectVPL buffer channels");
    test::require(buffer.data.size() == 26, "IndirectVPL buffer payload");

    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/indirect_vpl";
    const std::filesystem::path output_file = output_dir / (buffer.name + ".f32");
    writeFloatBuffer(output_file, buffer);
    std::ifstream input(output_file, std::ios::binary);
    std::string magic;
    std::string name;
    std::string dimensions;
    std::getline(input, magic);
    std::getline(input, name);
    std::getline(input, dimensions);

    test::require(magic == "HYPORASTER_F32_V1", "float buffer magic");
    test::require(name == "light_0_indirect_vpls", "float buffer name");
    test::require(dimensions == "2 1 13", "float buffer dimensions");
}

std::shared_ptr<Object> makeFloorObject(float half_size = 4.0f) {
    std::shared_ptr<Object> object = std::make_shared<Object>();
    object->addTriangle(Triangle(
        Vertex(Vec3f(-half_size, 0, -half_size), Vec3f(0, 1, 0)),
        Vertex(Vec3f(half_size, 0, half_size), Vec3f(0, 1, 0)),
        Vertex(Vec3f(half_size, 0, -half_size), Vec3f(0, 1, 0))
    ));
    object->addTriangle(Triangle(
        Vertex(Vec3f(-half_size, 0, -half_size), Vec3f(0, 1, 0)),
        Vertex(Vec3f(-half_size, 0, half_size), Vec3f(0, 1, 0)),
        Vertex(Vec3f(half_size, 0, half_size), Vec3f(0, 1, 0))
    ));
    return object;
}

void testRaySceneIntersectionHitsTriangle() {
    std::vector<std::shared_ptr<Object>> objects = {makeFloorObject()};
    RayHit hit = intersectSceneFirstHit(Vec3f(0, 1, 0), Vec3f(0, -1, 0), objects);

    test::require(hit.hit, "ray should hit floor");
    test::requireNear(hit.position.y(), 0.0f, 1e-5f, "hit y");
    test::require(hit.normal.y() > 0.9f, "hit normal should point up");
}

void testPointLightFirstBounceSamplerIsDeterministic() {
    std::vector<std::shared_ptr<Object>> objects = {makeFloorObject()};
    PointLight light(Vec3f(0, 1, 0), Vec3f(2, 2, 2));
    IndirectVPLConfig config;
    config.enabled = true;
    config.samples = 4;
    config.seed = 5;
    config.radius = 0.2f;

    std::vector<IndirectVPL> first = sampleFirstBounceIndirectVPLs(light, objects, config, 0);
    std::vector<IndirectVPL> second = sampleFirstBounceIndirectVPLs(light, objects, config, 0);

    test::require(first.size() == 4, "point light should produce requested first-bounce samples");
    test::require(second.size() == 4, "second point light sample count");
    for (size_t i = 0; i < first.size(); ++i) {
        test::require((first[i].position - second[i].position).norm() < 1e-6f, "sampler should be reproducible");
        test::require(first[i].normal.y() > 0.9f, "sample normal should come from floor");
        test::require(first[i].source_light_index == 0, "source light index should be preserved");
        test::requireNear(first[i].radius, 0.2f, 1e-6f, "sample radius should come from config");
    }
}

void testAreaAndDirectionalLightsGenerateFirstBounceSamples() {
    std::vector<std::shared_ptr<Object>> objects = {makeFloorObject()};
    IndirectVPLConfig config;
    config.enabled = true;
    config.samples = 4;
    config.radius = 0.3f;

    AreaLight area(Vec3f(0, 1, 0), Vec3f(2, 2, 2), Vec3f(0, -1, 0), Vec2f(1, 1), 4, 1, false);
    DirectionalLight sun(Vec3f(0, -1, 0), Vec3f(1, 1, 1));

    std::vector<IndirectVPL> area_samples = sampleFirstBounceIndirectVPLs(area, objects, config, 0);
    std::vector<IndirectVPL> sun_samples = sampleFirstBounceIndirectVPLs(sun, objects, config, 1);

    test::require(area_samples.size() == 4, "area light should produce first-bounce samples");
    test::require(sun_samples.size() == 4, "directional light should produce first-bounce samples");
    test::require(sun_samples[0].source_light_index == 1, "directional source light index");
}

void testSamplerHandlesEmptyGeometry() {
    std::vector<std::shared_ptr<Object>> objects;
    PointLight light(Vec3f(0, 1, 0), Vec3f(1, 1, 1));
    IndirectVPLConfig config;
    config.enabled = true;
    config.samples = 8;

    std::vector<IndirectVPL> samples = sampleFirstBounceIndirectVPLs(light, objects, config, 0);

    test::require(samples.empty(), "empty geometry should produce zero samples");
}

void writeSimpleFloorObj(const std::filesystem::path& path) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path);
    out << "v -4 0 -4\n";
    out << "v 4 0 4\n";
    out << "v 4 0 -4\n";
    out << "v -4 0 4\n";
    out << "vn 0 1 0\n";
    out << "f 1//1 2//1 3//1\n";
    out << "f 1//1 4//1 2//1\n";
}

std::string readBufferDimensions(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::string magic;
    std::string name;
    std::string dimensions;
    std::getline(input, magic);
    std::getline(input, name);
    std::getline(input, dimensions);
    return dimensions;
}

void testIndirectVPLContributionAndClamp() {
    IndirectVPL sample(
        Vec3f(0, 1, 0),
        Vec3f(0, -1, 0),
        Vec3f(4, 2, 1),
        1.0f,
        1,
        0,
        0
    );
    Vec3f radiance = evaluateIndirectVPLContribution(
        Vec3f(0, 0, 0),
        Vec3f(0, 1, 0),
        Vec3f(0.5f, 0.5f, 0.5f),
        sample,
        0.25f
    );

    test::require(radiance.x() > 0.0f, "indirect contribution should be positive");
    test::require(radiance.y() > 0.0f, "indirect contribution should be positive");
    test::require(radiance.z() > 0.0f, "indirect contribution should be positive");
    test::require(radiance.x() <= 0.25f + 1e-6f, "indirect contribution should clamp");
}

void testRasterizerExportsIndirectVPLDebugBuffer() {
    const std::filesystem::path output_dir = findProjectRoot() / "build/test_outputs/indirect_vpl_render";
    std::filesystem::remove_all(output_dir);
    std::filesystem::create_directories(output_dir);
    const std::filesystem::path obj_path = output_dir / "floor.obj";
    writeSimpleFloorObj(obj_path);
    const std::filesystem::path config_path = output_dir / "indirect_render.json";
    {
        std::ofstream out(config_path);
        out << "{\n";
        out << "  \"Camera\": {\"Resolution\": [8, 8], \"Position\": [0, 1, 3], \"Target\": [0, 0, 0], \"FocalLength\": 1, \"Fov\": 45},\n";
        out << "  \"Lights\": [{\"Type\": \"PointLight\", \"Position\": [0, 1, 0], \"Intensity\": [2, 2, 2], \"ShadowEnabled\": false,\n";
        out << "    \"IndirectVPL\": {\"Enabled\": true, \"Samples\": 4, \"Seed\": 1, \"Bounces\": 1, \"Visibility\": false, \"Clamp\": 1.0, \"Radius\": 0.2}}],\n";
        out << "  \"Materials\": [{\"Name\": \"mat\", \"Type\": \"ColorMat\", \"BaseColor\": [0.8, 0.8, 0.8], \"Shininess\": 8}],\n";
        out << "  \"Objects\": [{\"SourceFile\": \"" << obj_path.string() << "\", \"Translation\": [0, 0, 0], \"Rotation\": [0, 0, 0], \"Scale\": [1, 1, 1], \"Material\": \"mat\"}]\n";
        out << "}\n";
    }

    Rasterizer rasterizer(config_path.string());
    rasterizer.Pass();
    rasterizer.ExportGBuffer(output_dir / "gbuffer");
    rasterizer.ExportLightDebugData(output_dir / "shadow");

    const std::filesystem::path indirect_path = output_dir / "shadow/light_0_indirect_vpls.f32";
    test::require(std::filesystem::exists(indirect_path), "indirect VPL debug buffer should be exported");
    test::require(readBufferDimensions(indirect_path) == "4 1 13", "indirect VPL debug dimensions");

    const std::filesystem::path radiance_path = output_dir / "gbuffer/indirect_vpl_radiance.f32";
    test::require(std::filesystem::exists(radiance_path), "indirect radiance G-Buffer should be exported");
    test::require(readBufferDimensions(radiance_path) == "8 8 3", "indirect radiance dimensions");
}

}  // namespace

int main() {
    test::Runner runner;
    runner.run("SceneConfig parses IndirectVPL fields", testSceneConfigParsesIndirectVPLFields);
    runner.run("Legacy config parses IndirectVPL fields", testLegacyConfigParsesIndirectVPLFields);
    runner.run("IndirectVPL sample float record", testIndirectVPLSampleFloatRecord);
    runner.run("IndirectVPL float buffer contract", testIndirectVPLFloatBufferContract);
    runner.run("Ray scene intersection hits triangle", testRaySceneIntersectionHitsTriangle);
    runner.run("Point light first-bounce sampler is deterministic", testPointLightFirstBounceSamplerIsDeterministic);
    runner.run("Area and directional lights generate first-bounce samples", testAreaAndDirectionalLightsGenerateFirstBounceSamples);
    runner.run("Sampler handles empty geometry", testSamplerHandlesEmptyGeometry);
    runner.run("IndirectVPL contribution and clamp", testIndirectVPLContributionAndClamp);
    runner.run("Rasterizer exports IndirectVPL debug buffer", testRasterizerExportsIndirectVPLDebugBuffer);
    return runner.finish();
}
