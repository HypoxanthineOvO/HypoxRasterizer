#include "camera.hpp"

int main(int argc, char** argv) {
    // Construct Camera
    std::shared_ptr<Camera> camera
        = std::make_shared<Camera>();

    // Test Initialize
    camera->printCameraInfo();
    // Test MoveTo
    camera->moveTo(Vec3f(4, 2, 3));
    camera->printCameraInfo();
}