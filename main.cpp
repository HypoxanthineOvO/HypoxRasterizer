#include <iostream>
#include "rasterizer.hpp"

int main(int argc, char const *argv[]) {
    puts("==================== HypoxRasterizer ====================");
    // Create a Camera
    std::shared_ptr<Camera> cam = std::make_shared<Camera>(
        Vec3f(2, 1, -1), Vec3f(0, -1, 2)
    );
    // Create a Scene
    // Test scene: a triangle
    Vertex v0(Vec3f(0, 2, 0), Vec3f(1, 0, 0)),
        v1(Vec3f(0, 0, -2), Vec3f(1, 0, 0)),
        v2(Vec3f(0, 1, -2), Vec3f(1, 0, 0));
    Triangle tri(v0, v1, v2);
    Object obj;
    obj.addTriangle(tri);
    std::shared_ptr<Scene> scn = std::make_shared<Scene>();
    scn->addObject(obj);
    // Create a Rasterizer
    Rasterizer rast(cam, scn);

    // Pass the Rasterizer
    rast.Pass();
    return 0;
}