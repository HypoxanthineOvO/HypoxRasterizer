#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "utils.hpp"
#include "configs.hpp"

class Camera {
    // Extrinsic Parameters
    Vec3f position;
    Vec3f forward, right, up;

    // Intrinsic Parameters
    float focal_len;
    float fov;
    uint32_t width, height;
    float aspect_ratio;
public:
    /* Constructors */
    // Default Initialization
    Camera();
    Camera(Vec3f pos, Vec3f targ);
    Camera(Vec2i res, Vec3f pos, Vec3f targ);
    Camera(const CameraConfig& config);
    // Move Functions
    void moveTo(Vec3f pos);
    void lookAt(Vec3f target);

    // Get Matrix Functions
    Mat4f getViewMatrix() const;
    Mat4f getProjectionMatrix(bool isShadowMap = false) const;

    // Setters
    void setResolution(uint32_t h, uint32_t w) { height = h; width = w; }
    void setResolution(Vec2i res) { height = res.y(); width = res.x(); }
    void setFocalLength(float f) { focal_len = f; }
    void setFOV(float f) { fov = f; }

    // Getters
    Vec3f getPosition() const { return position; }
    Vec3f getForward() const { return forward; }
    Vec3f getRight() const { return right; }
    Vec3f getUp() const { return up; }
    float getFocalLength() const { return focal_len; }
    float getFOV() const { return fov; }
    uint32_t getWidth() const { return width; }
    uint32_t getHeight() const { return height; }
    Vec2i getResolution() const { return Vec2i(width, height); }
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
};



#endif // CAMERA_HPP_