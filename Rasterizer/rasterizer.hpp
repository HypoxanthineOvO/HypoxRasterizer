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
    
    /* Screen Space Buffer */
    std::vector<Vec3f> color_buffer;
    std::vector<float> depth_buffer;
};


#endif // RASTERIZER_HPP_