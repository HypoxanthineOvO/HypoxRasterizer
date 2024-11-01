#ifndef CONFIGS_HPP_
#define CONFIGS_HPP_

#include "cores.hpp"
#include <string>

typedef enum MaterialType {
    Color_Mat,
    Texture_Mat
} MaterialType;
typedef enum LightType {
    Point_Light
} LightType;

struct CameraConfig {
    Vec2i resolution;
    Vec3f position;
    Vec3f target;
    float focal_length;
    float fov;
};

struct MaterialConfig {
    std::string name;
    MaterialType type;
    Vec3f base_color;
    std::string texture_file_path;
};

struct ObjectConfig {
    std::string file_path;
    std::string material;
};

struct LightConfig {
    LightType type;
    // TODO: Other
};

class Config {
public:
    Config() = delete;
    Config(const std::string& config_file_path) {
        loadConfig(config_file_path);
    }
    void loadConfig(const std::string& config_file_path);

    // Sub-Configs
    CameraConfig camera_config;
    std::vector<LightConfig> lights_config;
    std::vector<MaterialConfig> materials_config;
    std::vector<ObjectConfig> objects_config;
};

#endif // CONFIGS_HPP_