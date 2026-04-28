#include "scene_config.hpp"

#include "nlohmann/json.hpp"
#include <fstream>
#include <stdexcept>

namespace {

const nlohmann::json& requireField(const nlohmann::json& value, const std::string& field) {
    if (!value.contains(field)) {
        throw std::runtime_error("Missing required field: " + field);
    }
    return value.at(field);
}

Vec2i loadVec2iV2(const nlohmann::json& value) {
    if (!value.is_array() || value.size() != 2) {
        throw std::runtime_error("Expected Vec2i array with 2 elements");
    }
    return Vec2i(value.at(0).get<int>(), value.at(1).get<int>());
}

Vec2f loadVec2fV2(const nlohmann::json& value) {
    if (!value.is_array() || value.size() != 2) {
        throw std::runtime_error("Expected Vec2f array with 2 elements");
    }
    return Vec2f(value.at(0).get<float>(), value.at(1).get<float>());
}

Vec3f loadVec3fV2(const nlohmann::json& value) {
    if (!value.is_array() || value.size() != 3) {
        throw std::runtime_error("Expected Vec3f array with 3 elements");
    }
    return Vec3f(value.at(0).get<float>(), value.at(1).get<float>(), value.at(2).get<float>());
}

SceneLightType parseLightType(const std::string& type) {
    if (type == "point") return SceneLightType::Point;
    if (type == "directional") return SceneLightType::Directional;
    if (type == "area") return SceneLightType::Area;
    if (type == "spot") return SceneLightType::Spot;
    throw std::runtime_error("Unknown light type: " + type);
}

std::vector<std::string> loadStringArray(const nlohmann::json& value) {
    if (!value.is_array()) {
        throw std::runtime_error("Expected string array");
    }
    std::vector<std::string> result;
    for (const auto& item : value) {
        result.push_back(item.get<std::string>());
    }
    return result;
}

std::vector<int> loadIntArrayV2(const nlohmann::json& value) {
    if (!value.is_array()) {
        throw std::runtime_error("Expected integer array");
    }
    std::vector<int> result;
    for (const auto& item : value) {
        result.push_back(item.get<int>());
    }
    return result;
}

IndirectVPLConfig loadIndirectVPLConfigV2(const nlohmann::json& value) {
    IndirectVPLConfig config;
    if (!value.is_object()) {
        throw std::runtime_error("indirectVPL must be an object");
    }
    if (value.contains("enabled")) config.enabled = value.at("enabled").get<bool>();
    if (value.contains("samples")) config.samples = value.at("samples").get<int>();
    if (value.contains("seed")) config.seed = value.at("seed").get<uint32_t>();
    if (value.contains("bounces")) config.bounces = value.at("bounces").get<int>();
    if (value.contains("visibility")) config.visibility = value.at("visibility").get<bool>();
    if (value.contains("clamp")) config.clamp = value.at("clamp").get<float>();
    if (value.contains("radius")) config.radius = value.at("radius").get<float>();
    if (value.contains("sourceLights")) config.source_light_indices = loadIntArrayV2(value.at("sourceLights"));
    return config;
}

}  // namespace

SceneConfig loadSceneConfig(const std::string& config_file_path) {
    std::ifstream raw_file(config_file_path);
    if (!raw_file.is_open()) {
        throw std::runtime_error("Cannot open scene config: " + config_file_path);
    }

    nlohmann::json raw;
    raw_file >> raw;

    SceneConfig scene;
    const auto& scene_node = requireField(raw, "scene");
    scene.name = requireField(scene_node, "name").get<std::string>();
    if (scene.name.empty()) {
        throw std::runtime_error("scene.name must not be empty");
    }

    const auto& camera = requireField(raw, "camera");
    scene.camera.resolution = loadVec2iV2(requireField(camera, "resolution"));
    scene.camera.position = loadVec3fV2(requireField(camera, "position"));
    scene.camera.target = loadVec3fV2(requireField(camera, "target"));
    if (camera.contains("focalLength")) {
        scene.camera.focal_length = camera.at("focalLength").get<float>();
    }
    scene.camera.fov = requireField(camera, "fov").get<float>();

    for (const auto& material : requireField(raw, "materials")) {
        SceneMaterialConfig item;
        item.id = requireField(material, "id").get<std::string>();
        item.type = requireField(material, "type").get<std::string>();
        if (material.contains("baseColor")) {
            item.base_color = loadVec3fV2(material.at("baseColor"));
        }
        if (material.contains("texture")) {
            item.texture_file = material.at("texture").get<std::string>();
        } else if (material.contains("textureFile")) {
            item.texture_file = material.at("textureFile").get<std::string>();
        } else if (material.contains("textureFilePath")) {
            item.texture_file = material.at("textureFilePath").get<std::string>();
        }
        if (material.contains("shininess")) {
            item.shininess = material.at("shininess").get<float>();
        }
        if (material.contains("roughness")) {
            item.roughness = material.at("roughness").get<float>();
        }
        if (material.contains("metallic")) {
            item.metallic = material.at("metallic").get<float>();
        }
        scene.materials.push_back(item);
    }

    for (const auto& object : requireField(raw, "objects")) {
        SceneObjectConfig item;
        item.name = requireField(object, "name").get<std::string>();
        item.mesh = requireField(object, "mesh").get<std::string>();
        item.material = requireField(object, "material").get<std::string>();
        if (object.contains("translation")) item.translation = loadVec3fV2(object.at("translation"));
        if (object.contains("rotation")) item.rotation = loadVec3fV2(object.at("rotation"));
        if (object.contains("scale")) item.scale = loadVec3fV2(object.at("scale"));
        scene.objects.push_back(item);
    }

    for (const auto& light : requireField(raw, "lights")) {
        SceneLightConfig item;
        item.name = requireField(light, "name").get<std::string>();
        item.type = parseLightType(requireField(light, "type").get<std::string>());
        if (light.contains("position")) item.position = loadVec3fV2(light.at("position"));
        if (light.contains("direction")) item.direction = loadVec3fV2(light.at("direction"));
        if (light.contains("normal")) item.normal = loadVec3fV2(light.at("normal"));
        if (light.contains("size")) item.size = loadVec2fV2(light.at("size"));
        if (light.contains("intensity")) item.intensity = loadVec3fV2(light.at("intensity"));
        if (light.contains("vpl")) {
            const auto& vpl = light.at("vpl");
            if (vpl.contains("samples")) item.vpl_sample_count = vpl.at("samples").get<int>();
            if (vpl.contains("seed")) item.vpl_seed = vpl.at("seed").get<uint32_t>();
            if (vpl.contains("stochastic")) item.vpl_stochastic = vpl.at("stochastic").get<bool>();
        }
        if (light.contains("shadow") && light.at("shadow").contains("enabled")) {
            item.shadow_enabled = light.at("shadow").at("enabled").get<bool>();
        }
        if (light.contains("indirectVPL")) {
            item.indirect_vpl = loadIndirectVPLConfigV2(light.at("indirectVPL"));
        } else if (light.contains("indirectVpl")) {
            item.indirect_vpl = loadIndirectVPLConfigV2(light.at("indirectVpl"));
        }
        scene.lights.push_back(item);
    }

    const auto& outputs = requireField(raw, "outputs");
    if (outputs.contains("root")) {
        scene.outputs.root = outputs.at("root").get<std::string>();
    }
    scene.outputs.buffers = loadStringArray(requireField(outputs, "buffers"));
    return scene;
}
