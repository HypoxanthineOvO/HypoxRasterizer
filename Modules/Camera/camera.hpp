#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "utils.hpp"

class Camera {
public:
    /* Constructors */
    // Default Initialization
    Camera();
    Camera(Vec3f pos, Vec3f targ);
    // Move Functions
    void moveTo(Vec3f pos);
    void lookAt(Vec3f target);

    // Get Matrix Functions
    Mat4f getViewMatrix();
    Mat4f getProjectionMatrix();

    // Setters
    void setFocalLength(float f) { focal_len = f; }
    void setFOV(float f) { fov = f; }

    // Getters
    Vec3f getPosition() { return position; }
    Vec3f getForward() { return forward; }
    Vec3f getRight() { return right; }
    Vec3f getUp() { return up; }
    float getFocalLength() { return focal_len; }
    float getFOV() { return fov; }
    void printCameraInfo() {
        printf("Position: ");
        utils::printVec(position);
        printf("Forward: ");
        utils::printVec(forward);
        printf("Right: ");
        utils::printVec(right);
        printf("Up: ");
        utils::printVec(up);
        printf("Focal Length: %f\n", focal_len);
        printf("FOV: %f\n", fov);
    }
private:
    // Extrinsic Parameters
    Vec3f position;
    Vec3f forward, right, up;

    // Intrinsic Parameters
    float focal_len;
    float fov;
};



#endif // CAMERA_HPP_