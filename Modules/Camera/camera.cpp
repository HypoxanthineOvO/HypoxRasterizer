#include "camera.hpp"

Camera::Camera():
    position(0, 0, 0),
    forward(0, 0, 1),
    right(1, 0, 0),
    up(0, 1, 0),
    focal_len(1),
    fov(60)
{}
//construct by parameters:
Camera::Camera(Vec3f pos, Vec3f target):
    position(pos),
    focal_len(1),
    fov(60){
    lookAt(target);
}

void Camera::moveTo(Vec3f pos) {
    position = pos;
}

void Camera::lookAt(Vec3f target) {
    forward = target - position;
    if(forward.norm() < LENGTH_EPS){
        printf("自己看自己可还行\n");
        exit(0);
    }
    else {
        forward = forward.normalized();
        if (utils::isequalVec(forward, REF_UP))
        {
            right = REF_RIGHT;
            up = right.cross(forward);
            up = up.normalized();
        }
        else if (utils::isequalVec(-forward, REF_UP)){
            right = -REF_RIGHT;
            up = right.cross(forward);
            up = up.normalized();
        }
        else{
            right = forward.cross(REF_UP);
            right = right.normalized();
            up = right.cross(forward);
            up = up.normalized();
        } 
    }
}

Mat4f Camera::getViewMatrix() {

}

Mat4f Camera::getProjectionMatrix() {

}