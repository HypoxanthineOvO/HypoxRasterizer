#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "utils.hpp"

class Camera {
public:
    /* Constructors */
    // Default Initialization
    Camera();

    // Move Functions
    void moveTo(Vec3f pos);
    void lookAt(Vec3f target);

    // Get Matrix Functions
    Mat4f getViewMatrix();
    Mat4f getProjectionMatrix();
private:
    // Extrinsic Parameters
    Vec3f position;
    Vec3f forward, right, up;

    // Intrinsic Parameters
    float focal_len;
    float fov;
};



#endif // CAMERA_HPP_