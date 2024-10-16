#include "camera.hpp"

Camera::Camera():
    position(0, 0, 0),
    forward(0, 0, 1),
    right(1, 0, 0),
    up(0, 1, 0),
    focal_len(1),
    fov(60)
{}

void Camera::moveTo(Vec3f pos) {
    position = pos;
}

void Camera::lookAt(Vec3f target) {
    // TODO
}