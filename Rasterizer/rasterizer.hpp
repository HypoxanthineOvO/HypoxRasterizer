#ifndef RASTERIZER_HPP_
#define RASTERIZER_HPP_


#include "camera.hpp"
#include "scene.hpp"

class Rasterizer {
public:
    /* Constructors */
    Rasterizer(std::shared_ptr<Camera> cam, std::shared_ptr<Scene> scn):
        camera(cam), scene(scn) {
            // TODO: Initialize Buffers.
            triangle_buffer.clear();

            // Initialize the Screen Space Buffer
            color_buffer.resize(camera->getWidth() * camera->getHeight());
            depth_buffer.resize(camera->getWidth() * camera->getHeight());
    }

    // Pass
    void Pass();
    void VertexProcessing();
    void TriangleProcessing();
    void Rasterization();
    void FragmentProcessing();
    void DisplayToImage();

private:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Scene> scene;

    /* Triangle Buffer */
    std::vector<Triangle> triangle_buffer;
    
    /* Screen Space Buffer */
    std::vector<Vec3f> color_buffer;
    std::vector<float> depth_buffer;
};


#endif // RASTERIZER_HPP_