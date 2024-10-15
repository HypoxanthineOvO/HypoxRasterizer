#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "utils.hpp"

class Camera {
public:

private:
    // Extrinsic Parameters
    Vec3f position;
    Vec3f forward, right, up;

    // Intrinsic Parameters
    float focal_len;
    float fov;
};



#endif // CAMERA_HPP_