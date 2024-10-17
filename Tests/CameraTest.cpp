#include "camera.hpp"

int main(int argc, char** argv) {
    // Construct Camera
    std::shared_ptr<Camera> camera
        = std::make_shared<Camera>();

    // Test Initialize
    puts("============================Test Initialize===============================");
    camera->printCameraInfo();
    // Test MoveTo
    puts("===============================Test MoveTo===============================");
    camera->moveTo(Vec3f(4, 2, 3));
    camera->printCameraInfo();
    // Test lookAt
    puts("===============================Test lookAt===============================");
    puts("Forward == ref_up cases:");
    camera->lookAt(Vec3f(4, 2, 4));
    camera->printCameraInfo();
    puts("---------------------------------------------------------------------------");
    camera->lookAt(Vec3f(4, 2, 2));
    camera->printCameraInfo();
    puts("---------------------------------------------------------------------------");
    puts("Forward != ref_up cases:");
    camera->lookAt(Vec3f(1, 2, 2));
    camera->printCameraInfo();
    puts("---------------------------------------------------------------------------");
    puts("target == position cases:");
    camera->lookAt(Vec3f(4, 2, 3));
    camera->printCameraInfo();
}