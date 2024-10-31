#include <iostream>
#include "rasterizer.hpp"

int main(int argc, char const *argv[]) {
    puts("==================== HypoxRasterizer ====================");
    // Create a Camera
    std::shared_ptr<Camera> cam = std::make_shared<Camera>(
        Vec3f(4, 0, -1), Vec3f(2, 0, -1)
    );
    cam->setResolution(800, 800);
    // Create a Scene
    std::shared_ptr<Scene> scn = std::make_shared<Scene>();

    // std::shared_ptr<Object> obj = std::make_shared<Object>(
    //     "./assets/Money/2.obj"  
    // );
    // Mat4f trans_matrix = utils::generateModelMatrix(
    //     Vec3f(0, 0, 0), // Translation
    //     Vec3f(0, 0, 0), // Rotation
    //     Vec3f(0.08, 0.08, 0.08)
    // );
    // std::shared_ptr<Materials> mat = std::make_shared<TextureMaterial>(
    //     "./assets/Money/2.png"
    // );
    std::shared_ptr<Object> obj = std::make_shared<Object>(
        "./assets/Bunny/bunny.obj"    
    );
    Mat4f trans_matrix = utils::generateModelMatrix(
        Vec3f(0.1, -2, -1), // Translation
        Vec3f(0, 140, 20), // Rotation
        Vec3f(5, 5, 5)
    );
    // std::shared_ptr<Object> obj = std::make_shared<Object>(
    //     "./assets/Objects/sphere.obj"    
    // );
    // Mat4f trans_matrix = utils::generateModelMatrix(
    //     Vec3f(0, 0, 0), // Translation
    //     Vec3f(0, 140, 20), // Rotation
    //     Vec3f(2, 2, 2)
    // );

    
    std::shared_ptr<Materials> mat = std::make_shared<ColorMaterial>(Vec3f(0.8, 0.3, 0.8), 5.0f);
    
    obj->localToWorld(trans_matrix);
    obj->setMaterial(mat);
    scn->addObject(obj);
    // Test scene: a light source
    std::shared_ptr<Light> light = std::make_shared<PointLight>(
        Vec3f(8, 3, -1), Vec3f(1, 1, 1)
    );
    scn->addLight(light);
    // Create a Rasterizer
    Rasterizer rast(cam, scn);

    // Pass the Rasterizer
    rast.Pass();
    return 0;
}