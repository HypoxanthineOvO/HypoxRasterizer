#include <iostream>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include "cli.hpp"
#include "configs.hpp"
#include "nlohmann/json.hpp"
#include "output_manager.hpp"
#include "rasterizer.hpp"

std::string config_path;

namespace {

std::string sceneNameFromConfigPath(const std::string& path) {
    std::filesystem::path config(path);
    std::string stem = config.stem().string();
    return stem.empty() ? "scene" : stem;
}

bool wantsBuffer(const std::vector<std::string>& buffers, const std::string& name) {
    return buffers.empty() || std::find(buffers.begin(), buffers.end(), name) != buffers.end();
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.rfind(prefix, 0) == 0;
}

void cleanupLegacyRendererOutputs(const std::filesystem::path& working_dir) {
    std::filesystem::remove(working_dir / "color.png");
    std::filesystem::remove(working_dir / "depth.png");
    std::filesystem::remove(working_dir / "normal.png");

    for (const auto& entry : std::filesystem::directory_iterator(working_dir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const std::string name = entry.path().filename().string();
        if (startsWith(name, "PointlightShadowMap") ||
            startsWith(name, "DirectionalLightShadowMap") ||
            startsWith(name, "ArealightShadowMap")) {
            std::filesystem::remove(entry.path());
        }
    }
}

std::string sceneNameForOutput(const std::string& path) {
    std::ifstream input(path);
    if (input.is_open()) {
        try {
            nlohmann::json raw;
            input >> raw;
            if (raw.contains("scene") && raw.at("scene").contains("name")) {
                return raw.at("scene").at("name").get<std::string>();
            }
        } catch (const std::exception&) {
        }
    }
    return sceneNameFromConfigPath(path);
}

nlohmann::json convertV2ToLegacyConfig(const nlohmann::json& raw) {
    nlohmann::json legacy;
    const auto& camera = raw.at("camera");
    legacy["Camera"]["Resolution"] = camera.at("resolution");
    legacy["Camera"]["Position"] = camera.at("position");
    legacy["Camera"]["Target"] = camera.at("target");
    legacy["Camera"]["FocalLength"] = camera.value("focalLength", 1.0f);
    legacy["Camera"]["Fov"] = camera.at("fov");

    legacy["Materials"] = nlohmann::json::array();
    for (const auto& material : raw.at("materials")) {
        nlohmann::json item;
        item["Name"] = material.at("id");
        const std::string material_type = material.value("type", "phong");
        if (material_type == "texture" || material.contains("texture") ||
            material.contains("textureFile") || material.contains("textureFilePath")) {
            item["Type"] = "TextureMat";
            if (material.contains("texture")) {
                item["TextureFilePath"] = material.at("texture");
            } else if (material.contains("textureFile")) {
                item["TextureFilePath"] = material.at("textureFile");
            } else {
                item["TextureFilePath"] = material.at("textureFilePath");
            }
        } else {
            item["Type"] = "ColorMat";
            item["BaseColor"] = material.value("baseColor", nlohmann::json::array({1, 1, 1}));
        }
        item["Shininess"] = material.value("shininess", 15.0f);
        item["Roughness"] = material.value("roughness", 0.5f);
        item["Metallic"] = material.value("metallic", 0.0f);
        legacy["Materials"].push_back(item);
    }

    legacy["Objects"] = nlohmann::json::array();
    for (const auto& object : raw.at("objects")) {
        nlohmann::json item;
        item["SourceFile"] = object.at("mesh");
        item["Translation"] = object.value("translation", nlohmann::json::array({0, 0, 0}));
        item["Rotation"] = object.value("rotation", nlohmann::json::array({0, 0, 0}));
        item["Scale"] = object.value("scale", nlohmann::json::array({1, 1, 1}));
        item["Material"] = object.at("material");
        legacy["Objects"].push_back(item);
    }

    legacy["Lights"] = nlohmann::json::array();
    for (const auto& light : raw.at("lights")) {
        const std::string type = light.at("type").get<std::string>();
        nlohmann::json item;
        if (type == "point") {
            item["Type"] = "PointLight";
            item["Position"] = light.at("position");
        } else if (type == "directional") {
            item["Type"] = "DirectionalLight";
            item["Direction"] = light.at("direction");
        } else if (type == "area") {
            item["Type"] = "AreaLight";
            item["Position"] = light.at("position");
            item["Normal"] = light.at("normal");
            item["Size"] = light.at("size");
            if (light.contains("vpl")) {
                const auto& vpl = light.at("vpl");
                item["VplSampleCount"] = vpl.value("samples", NUM_SQRT_DIRECT_VPL * NUM_SQRT_DIRECT_VPL);
                item["VplSeed"] = vpl.value("seed", 0);
                item["VplStochastic"] = vpl.value("stochastic", false);
            }
        } else {
            throw std::runtime_error("Unsupported v2 light type for renderer adapter: " + type);
        }
        item["Intensity"] = light.at("intensity");
        item["ShadowEnabled"] = true;
        if (light.contains("shadow") && light.at("shadow").contains("enabled")) {
            item["ShadowEnabled"] = light.at("shadow").at("enabled");
        }
        if (light.contains("indirectVPL")) {
            const auto& indirect = light.at("indirectVPL");
            item["IndirectVPL"]["Enabled"] = indirect.value("enabled", false);
            item["IndirectVPL"]["Samples"] = indirect.value("samples", 0);
            item["IndirectVPL"]["Seed"] = indirect.value("seed", 0);
            item["IndirectVPL"]["Bounces"] = indirect.value("bounces", 1);
            item["IndirectVPL"]["Visibility"] = indirect.value("visibility", true);
            item["IndirectVPL"]["Clamp"] = indirect.value("clamp", 10.0f);
            item["IndirectVPL"]["Radius"] = indirect.value("radius", 1.0f);
            item["IndirectVPL"]["SourceLights"] = indirect.value("sourceLights", nlohmann::json::array());
        }
        legacy["Lights"].push_back(item);
    }
    return legacy;
}

std::string materializeRenderConfig(const std::string& path, const CliOptions& options) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Cannot open config file: " + path);
    }

    nlohmann::json raw;
    input >> raw;

    bool needs_temp_config = options.has_resolution_override || raw.contains("scene");
    if (!needs_temp_config) {
        return path;
    }

    nlohmann::json render_config = raw.contains("scene") ? convertV2ToLegacyConfig(raw) : raw;
    if (options.has_resolution_override) {
        render_config["Camera"]["Resolution"] = {
            options.resolution_override.x(),
            options.resolution_override.y()
        };
    }

    std::filesystem::path output_dir = "build/cli_overrides";
    std::filesystem::create_directories(output_dir);
    std::filesystem::path output_file = output_dir / (sceneNameFromConfigPath(path) + ".legacy.json");
    std::ofstream output(output_file);
    output << render_config.dump(4);
    return output_file.string();
}

}  // namespace

int main(int argc, char const *argv[]) {
    puts("==================== HypoxRasterizer ====================");
    CliOptions options;
    try {
        options = parseCliOptions(argc, argv);
    } catch (const std::exception& ex) {
        std::cerr << "CLI error: " << ex.what() << std::endl;
        return 1;
    }

    if (!options.config_path.empty()) {
        config_path = options.config_path;
    } else {
        std::cout << "Please provide the config file path: ";
        std::cin >> config_path;
    }
    
    try {
        std::string render_config_path = materializeRenderConfig(config_path, options);
        cleanupLegacyRendererOutputs(std::filesystem::current_path());
        // Create a Rasterizer
        Rasterizer rast(render_config_path);
        

        // Pass the Rasterizer
        rast.Pass();
        OutputManager outputs(options.output_root, sceneNameForOutput(config_path));
        outputs.prepare(options.buffers);
        if (wantsBuffer(options.buffers, "gbuffer")) {
            rast.ExportGBuffer(outputs.gbufferDir());
        }
        if (wantsBuffer(options.buffers, "shadow")) {
            rast.ExportLightDebugData(outputs.shadowDir());
        }
        outputs.publishLegacyRendererOutputs(options.buffers);
    } catch (const std::exception& ex) {
        std::cerr << "Render error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
