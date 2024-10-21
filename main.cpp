#include <iostream>
#include "rasterizer.hpp"

int main(int argc, char const *argv[]) {
    puts("==================== HypoxRasterizer ====================");
    // Create a Camera
    std::shared_ptr<Camera> cam = std::make_shared<Camera>(
        Vec3f(2, 1, -1), Vec3f(0, 1, -2)
    );
    // Create a Scene
    std::shared_ptr<Scene> scn = std::make_shared<Scene>();
    // Test scene: a triangle
    Vertex v0(Vec3f(0, 2, 0), Vec3f(1, 0, 0)),
        v1(Vec3f(0, 0, -2), Vec3f(1, 0, 0)),
        v2(Vec3f(0, 1, -2), Vec3f(1, 0, 0));
    Triangle tri(v0, v1, v2);
    std::shared_ptr<Object> obj = std::make_shared<Object>();
    obj->addTriangle(tri);
    scn->addObject(obj);
    // Test scene: a light source
    std::shared_ptr<Light> light = std::make_shared<PointLight>(
        Vec3f(2, 3, -1), Vec3f(1, 0.5, 0.5)
    );
    scn->addLight(light);
    // Create a Rasterizer
    Rasterizer rast(cam, scn);

    // Pass the Rasterizer
    rast.Pass();
    return 0;
}