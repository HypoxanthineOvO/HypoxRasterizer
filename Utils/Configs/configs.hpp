#ifndef CONFIGS_HPP_
#define CONFIGS_HPP_

#include "cores.hpp"
#include "indirect_vpl_config.hpp"
#include <cstdint>
#include <string>

typedef enum MaterialType {
    Color_Mat,
    Texture_Mat
} MaterialType;
typedef enum LightType {
    Point_Light,
    Directional_Light,
    Area_Light
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
    float shininess;
    float roughness = 0.5f;
    float metallic = 0.0f;
    Vec3f base_color;
    std::string texture_file_path;
};

struct ObjectConfig {
    std::string file_path;
    Vec3f translation, rotation, scale;
    std::string material;
};

struct LightConfig {
    LightType type;
    
    // Point Light
    Vec3f position;
    Vec3f intensity;

    // Directional Light
    Vec3f direction = Vec3f(0, -1, 0);

    // Area Light
    Vec3f normal;
    Vec2f size;
    int vpl_sample_count = NUM_SQRT_DIRECT_VPL * NUM_SQRT_DIRECT_VPL;
    uint32_t vpl_seed = 0;
    bool vpl_stochastic = false;
    bool shadow_enabled = true;
    IndirectVPLConfig indirect_vpl;
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
