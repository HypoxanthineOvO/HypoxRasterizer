#ifndef SHADOWMAP_HPP_
#define SHADOWMAP_HPP_

#include "utils.hpp"
#include "camera.hpp"
#include "object.hpp"

class ShadowMap {
public:
    // Constructors
    ShadowMap(): resolution(DEFAULT_SHADOW_MAP_RESOLUTION, DEFAULT_SHADOW_MAP_RESOLUTION),
        depth_buffer(resolution.x() * resolution.y(), 1), depth_buffer_tofile(resolution.x() * resolution.y(), 1) {}
    ShadowMap(int res): resolution(res, res),
        depth_buffer(resolution.x() * resolution.y(), 1), depth_buffer_tofile(resolution.x() * resolution.y(), 1)  {}
    ShadowMap(int x, int y): resolution(x, y),
        depth_buffer(resolution.x() * resolution.y(), 1), depth_buffer_tofile(resolution.x() * resolution.y(), 1)  {}
    ShadowMap(Vec2i res): resolution(res),
        depth_buffer(resolution.x() * resolution.y(), 1), depth_buffer_tofile(resolution.x() * resolution.y(), 1)  {}
    void initialize(Vec3f position, Vec3f direction, float fov = 90) {
        Vec3f target = position + direction;
        camera = std::make_shared<Camera>(position, target);
        camera->setFOV(fov);
        camera->setFocalLength(0.1);
        camera->setResolution(resolution);
    }

    void generateDepthBuffer(std::vector<std::shared_ptr<Object>>& objects);

    bool isLighted(Vec3f position) const;

    void showShadowMap(const std::string& file_name);
private:
    Vec2i resolution;

    std::shared_ptr<Camera> camera;
    std::vector<float> depth_buffer;
    std::vector<float> depth_buffer_tofile;
};

#endif // SHADOWMAP_HPP_