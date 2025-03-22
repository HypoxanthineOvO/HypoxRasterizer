#ifndef RASTERIZER_HPP_
#define RASTERIZER_HPP_


#include "camera.hpp"
#include "scene.hpp"
#include "configs.hpp"

class Rasterizer {
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;

    /* Triangle Buffer */
    std::vector<Triangle> triangle_buffer;
    std::vector<Triangle> org_triangle_buffer;
    /* Screen Space Buffer */
    std::vector<Vec3f> color_buffer;
    std::vector<float> depth_buffer;
    std::vector<Vec3f> position_buffer;
    std::vector<Vec3f> org_position_buffer;
    std::vector<Vec3f> normal_buffer;
    std::vector<Vec3f> org_normal_buffer;
    std::vector<Vec2f> uv_buffer;
    std::vector<std::shared_ptr<Materials>> material_buffer;
public:
    /* Constructors */
    Rasterizer(std::shared_ptr<Camera> cam, std::shared_ptr<Scene> scn):
        camera(cam), scene(scn) {
            // TODO: Initialize Buffers.
            triangle_buffer.clear();

            // Initialize the Screen Space Buffer with -
            color_buffer.resize(camera->getWidth() * camera->getHeight(), Vec3f::Zero());
            depth_buffer.resize(camera->getWidth() * camera->getHeight(), 1);
            position_buffer.resize(camera->getWidth() * camera->getHeight(), Vec3f::Zero());
            normal_buffer.resize(camera->getWidth() * camera->getHeight(), Vec3f::Zero());
            uv_buffer.resize(camera->getWidth() * camera->getHeight(), -Vec2f::Ones());
            material_buffer.resize(camera->getWidth() * camera->getHeight(), nullptr);
    }
    Rasterizer(const std::string& config_path);

    void initializeFromConfig(const Config& config);

    // Pass
    void Pass();
    void VertexProcessing();
    void FragmentProcessing();
    void FragmentShading();
    void DisplayToImage();
};


#endif // RASTERIZER_HPP_