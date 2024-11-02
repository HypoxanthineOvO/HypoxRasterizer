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

Vec3f loadVec3f(const nlohmann::json& j) {
    float x, y, z;
    j[0].get_to(x), j[1].get_to(y), j[2].get_to(z);
    return Vec3f(x, y, z);
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
        else {
            puts("Unknown Light Type");
            exit(1);
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