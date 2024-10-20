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
    // puts("target == position cases:");
    // camera1->lookAt(Vec3f(4, 2, 3));
    // camera1->printCameraInfo();
    puts("===============================Test getViewMatrix===============================");
    Mat4f viewmat = camera1->getViewMatrix();
    //std::cout << "View matrix: " << std::endl;
    //std::cout << viewmat << std::endl;
    printf("View matrix:\n");
    utils::printMat(viewmat);
    Vec3f pos = camera1->getPosition();
    Vec3f forw = camera1->getForward();
    Vec3f up = camera1->getUp();
    Vec3f right = camera1->getRight();
    Vec3f object_for = pos + forw;
    Vec3f object_up = pos + up;
    Vec3f object_rig = pos + right;
    Vec4f forw_obj_pos,up_obj_pos,right_obj_pos;
    forw_obj_pos << object_for, 1 ;
    up_obj_pos << object_up, 1;
    right_obj_pos << object_rig, 1;
    //std::cout<<"Forward:\n" << viewmat * forw_obj_pos << std::endl;
    //std::cout<< "Up:\n" << viewmat * up_obj_pos << std::endl;
    //std::cout<< "Right:\n" << viewmat * right_obj_pos << std::endl;
    printf("Forward: ");
    utils::printVec(viewmat * forw_obj_pos);
    printf("Up: ");
    utils::printVec(viewmat * up_obj_pos);
    printf("Right: ");
    utils::printVec(viewmat * right_obj_pos);

    puts("===============================Test getProjectionMatrix===============================");
    Mat4f projmat = camera1->getProjectionMatrix();
    Vec4f afterproj = projmat * Vec4f(0.5, 0.5, -1, 1);
    //std::cout << "Forward:\n"
    //          << afterproj / afterproj(3) << std::endl;
    printf("Forward: ");
    utils::printVec(afterproj / afterproj(3));
}