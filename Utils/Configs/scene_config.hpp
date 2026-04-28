#ifndef SCENE_CONFIG_HPP_
#define SCENE_CONFIG_HPP_

#include "cores.hpp"
#include "indirect_vpl_config.hpp"
#include <cstdint>
#include <string>
#include <vector>

enum class SceneLightType {
    Point,
    Directional,
    Area,
    Spot
};

struct SceneCameraConfig {
    Vec2i resolution = Vec2i(0, 0);
    Vec3f position = Vec3f::Zero();
    Vec3f target = Vec3f(0, 0, -1);
    float focal_length = 1.0f;
    float fov = 45.0f;
};

struct SceneMaterialConfig {
    std::string id;
    std::string type;
    Vec3f base_color = Vec3f(1, 1, 1);
    std::string texture_file;
    float shininess = 15.0f;
    float roughness = 0.5f;
    float metallic = 0.0f;
};

struct SceneObjectConfig {
    std::string name;
    std::string mesh;
    std::string material;
    Vec3f translation = Vec3f::Zero();
    Vec3f rotation = Vec3f::Zero();
    Vec3f scale = Vec3f(1, 1, 1);
};

struct SceneLightConfig {
    std::string name;
    SceneLightType type = SceneLightType::Point;
    Vec3f position = Vec3f::Zero();
    Vec3f direction = Vec3f(0, -1, 0);
    Vec3f normal = Vec3f(0, -1, 0);
    Vec2f size = Vec2f(1, 1);
    Vec3f intensity = Vec3f(1, 1, 1);
    int vpl_sample_count = 100;
    uint32_t vpl_seed = 0;
    bool vpl_stochastic = false;
    bool shadow_enabled = false;
    IndirectVPLConfig indirect_vpl;
};

struct SceneOutputConfig {
    std::string root = "outputs";
    std::vector<std::string> buffers;
};

struct SceneConfig {
    std::string name;
    SceneCameraConfig camera;
    std::vector<SceneMaterialConfig> materials;
    std::vector<SceneObjectConfig> objects;
    std::vector<SceneLightConfig> lights;
    SceneOutputConfig outputs;
};

SceneConfig loadSceneConfig(const std::string& config_file_path);

#endif  // SCENE_CONFIG_HPP_
