#include "configs.hpp"
#include <fstream>
#include "nlohmann/json.hpp"

float loadFloat(const nlohmann::json& j) {
    float v;
    j.get_to(v);
    return v;
}

Vec2i loadVec2i(const nlohmann::json& j) {
    int x, y;
    j[0].get_to(x), j[1].get_to(y);
    return Vec2i(x, y);
}

Vec2f loadVec2f(const nlohmann::json& j) {
    float x, y;
    j[0].get_to(x), j[1].get_to(y);
    return Vec2f(x, y);
}

Vec3f loadVec3f(const nlohmann::json& j) {
    float x, y, z;
    j[0].get_to(x), j[1].get_to(y), j[2].get_to(z);
    return Vec3f(x, y, z);
}

std::vector<int> loadIntArray(const nlohmann::json& j) {
    std::vector<int> result;
    if (!j.is_array()) {
        return result;
    }
    for (const auto& item : j) {
        result.push_back(item.get<int>());
    }
    return result;
}

IndirectVPLConfig loadLegacyIndirectVPLConfig(const nlohmann::json& j) {
    IndirectVPLConfig config;
    if (!j.is_object()) {
        return config;
    }
    if (j.contains("Enabled")) {
        config.enabled = j["Enabled"].get<bool>();
    } else if (j.contains("enabled")) {
        config.enabled = j["enabled"].get<bool>();
    }
    if (j.contains("Samples")) {
        config.samples = j["Samples"].get<int>();
    } else if (j.contains("samples")) {
        config.samples = j["samples"].get<int>();
    }
    if (j.contains("Seed")) {
        config.seed = j["Seed"].get<uint32_t>();
    } else if (j.contains("seed")) {
        config.seed = j["seed"].get<uint32_t>();
    }
    if (j.contains("Bounces")) {
        config.bounces = j["Bounces"].get<int>();
    } else if (j.contains("bounces")) {
        config.bounces = j["bounces"].get<int>();
    }
    if (j.contains("Visibility")) {
        config.visibility = j["Visibility"].get<bool>();
    } else if (j.contains("visibility")) {
        config.visibility = j["visibility"].get<bool>();
    }
    if (j.contains("Clamp")) {
        config.clamp = j["Clamp"].get<float>();
    } else if (j.contains("clamp")) {
        config.clamp = j["clamp"].get<float>();
    }
    if (j.contains("Radius")) {
        config.radius = j["Radius"].get<float>();
    } else if (j.contains("radius")) {
        config.radius = j["radius"].get<float>();
    }
    if (j.contains("SourceLights")) {
        config.source_light_indices = loadIntArray(j["SourceLights"]);
    } else if (j.contains("sourceLights")) {
        config.source_light_indices = loadIntArray(j["sourceLights"]);
    }
    return config;
}

void Config::loadConfig(const std::string& config_file_path) {
    std::ifstream raw_file(config_file_path);
    nlohmann::json raw;
    raw_file >> raw;

    puts("Loading Config...");

    // Load Camera Config
    puts("Loading Camera Config...");
    camera_config.resolution = loadVec2i(raw["Camera"]["Resolution"]);
    camera_config.position = loadVec3f(raw["Camera"]["Position"]);
    camera_config.target = loadVec3f(raw["Camera"]["Target"]);
    camera_config.focal_length = loadFloat(raw["Camera"]["FocalLength"]);
    camera_config.fov = loadFloat(raw["Camera"]["Fov"]);
    puts("Camera Config Loaded Successfully!");

    // Load Lights Config
    puts("Loading Lights Config...");
    for (auto& light : raw["Lights"]) {
        LightConfig l;
        if (light["Type"] == "PointLight") {
            l.type = LightType::Point_Light;
            l.position = loadVec3f(light["Position"]);
            l.intensity = loadVec3f(light["Intensity"]);
        }
        else if (light["Type"] == "DirectionalLight") {
            l.type = LightType::Directional_Light;
            l.direction = loadVec3f(light["Direction"]);
            l.intensity = loadVec3f(light["Intensity"]);
        }
        else if (light["Type"] == "AreaLight") {
            l.type = LightType::Area_Light;
            l.position = loadVec3f(light["Position"]);
            l.intensity = loadVec3f(light["Intensity"]);
            l.normal = loadVec3f(light["Normal"]);
            l.size = loadVec2f(light["Size"]);
            if (light.contains("VplSampleCount")) {
                l.vpl_sample_count = light["VplSampleCount"].get<int>();
            } else if (light.contains("vplSampleCount")) {
                l.vpl_sample_count = light["vplSampleCount"].get<int>();
            }
            if (light.contains("VplSeed")) {
                l.vpl_seed = light["VplSeed"].get<uint32_t>();
            } else if (light.contains("vplSeed")) {
                l.vpl_seed = light["vplSeed"].get<uint32_t>();
            }
            if (light.contains("VplStochastic")) {
                l.vpl_stochastic = light["VplStochastic"].get<bool>();
            } else if (light.contains("vplStochastic")) {
                l.vpl_stochastic = light["vplStochastic"].get<bool>();
            }
        }
        else {
            puts("Unknown Light Type");
            exit(1);
        }
        if (light.contains("ShadowEnabled")) {
            l.shadow_enabled = light["ShadowEnabled"].get<bool>();
        } else if (light.contains("shadowEnabled")) {
            l.shadow_enabled = light["shadowEnabled"].get<bool>();
        }
        if (light.contains("IndirectVPL")) {
            l.indirect_vpl = loadLegacyIndirectVPLConfig(light["IndirectVPL"]);
        } else if (light.contains("indirectVPL")) {
            l.indirect_vpl = loadLegacyIndirectVPLConfig(light["indirectVPL"]);
        }
        lights_config.push_back(l);
    }
    puts("Lights Config Loaded Successfully!");

    // Load Materials Config
    puts("Loading Materials Config...");
    for (auto& mat : raw["Materials"]) {
        MaterialConfig m;
        m.name = mat["Name"];
        if (mat["Type"] == "ColorMat") {
            m.type = MaterialType::Color_Mat;
            m.base_color = loadVec3f(mat["BaseColor"]);
        }
        else if (mat["Type"] == "TextureMat") {
            m.type = MaterialType::Texture_Mat;
            m.texture_file_path = mat["TextureFilePath"];
        }
        m.shininess = loadFloat(mat["Shininess"]);
        if (mat.contains("Roughness")) {
            m.roughness = loadFloat(mat["Roughness"]);
        } else if (mat.contains("roughness")) {
            m.roughness = loadFloat(mat["roughness"]);
        }
        if (mat.contains("Metallic")) {
            m.metallic = loadFloat(mat["Metallic"]);
        } else if (mat.contains("metallic")) {
            m.metallic = loadFloat(mat["metallic"]);
        }
        materials_config.push_back(m);
    }
    puts("Materials Config Loaded Successfully!");

    // Load Objects Config
    puts("Loading Objects Config...");
    for (auto& obj : raw["Objects"]) {
        ObjectConfig o;
        o.file_path = obj["SourceFile"];
        o.translation = loadVec3f(obj["Translation"]);
        o.rotation = loadVec3f(obj["Rotation"]);
        o.scale = loadVec3f(obj["Scale"]);
        o.material = obj["Material"];
        objects_config.push_back(o);
    }
    puts("Objects Config Loaded Successfully!");

    raw_file.close();

    puts("Config Loaded Successfully!");
}
