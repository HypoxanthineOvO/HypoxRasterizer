#include "camera.hpp"

int main(int argc, char** argv) {
    // Construct Camera
    std::shared_ptr<Camera> camera1
        = std::make_shared<Camera>();
    std::shared_ptr<Camera> camera2 
        = std::make_shared<Camera>(Vec3f(0, 1, 0), Vec3f(3, 6, 7));
    // Test Default Initialize
    puts("========================Test Edfault Initialize===========================");
    camera1->printCameraInfo();
    //Test Parameteric Initialize
    puts("========================Test Edfault Initialize===========================");
    camera2->printCameraInfo();
    // Test MoveTo
    puts("===============================Test MoveTo===============================");
    camera1->moveTo(Vec3f(4, 2, 3));
    camera1->printCameraInfo();
    // Test lookAt
    puts("===============================Test lookAt===============================");
    puts("Forward == ref_up cases:");
    camera1->lookAt(Vec3f(4, 2, 4));
    camera1->printCameraInfo();
    puts("---------------------------------------------------------------------------");
    camera1->lookAt(Vec3f(4, 2, 2));
    camera1->printCameraInfo();
    puts("---------------------------------------------------------------------------");
    puts("Forward != ref_up cases:");
    camera1->lookAt(Vec3f(1, 2, 2));
    camera1->printCameraInfo();
    puts("---------------------------------------------------------------------------");
    puts("target == position cases:");
    camera1->lookAt(Vec3f(4, 2, 3));
    camera1->printCameraInfo();
}