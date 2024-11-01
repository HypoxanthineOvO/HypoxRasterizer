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

}