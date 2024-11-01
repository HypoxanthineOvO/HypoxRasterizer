#include "camera.hpp"

Camera::Camera():
    position(0, 0, 0),
    forward(0, 0, -1),
    right(1, 0, 0),
    up(0, 1, 0),
    focal_len(1),
    fov(DEFAULT_FOV),
    width(DEFAULT_WIDTH),
    height(DEFAULT_HEIGHT)
{}
//construct by parameters:
Camera::Camera(Vec3f pos, Vec3f target):
    position(pos),
    focal_len(1),
    fov(DEFAULT_FOV),
    width(DEFAULT_WIDTH),
    height(DEFAULT_HEIGHT)
    {
    lookAt(target);
}

Camera::Camera(Vec2i res, Vec3f pos, Vec3f targ):
    position(pos),
    focal_len(1),
    fov(DEFAULT_FOV),
    width(res.x()),
    height(res.y())
{
    lookAt(targ);
}

Camera::Camera(const CameraConfig& config):
    position(config.position),
    focal_len(config.focal_length),
    fov(config.fov),
    width(config.resolution.x()),
    height(config.resolution.y())
{
    lookAt(config.target);
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
    Mat4f T_camera_inv,R_camera_T ;
    // Right-handed coordinate system
    T_camera_inv << 1, 0, 0, -position.x(),
                0, 1, 0, -position.y(),
                0, 0, 1, -position.z(),
                0, 0, 0, 1;
    R_camera_T << right.x(), right.y(), right.z(), 0,
                up.x(), up.y(), up.z(), 0,
                -forward.x(), -forward.y(), -forward.z(), 0,           
                0, 0, 0, 1;
    Mat4f view_matrix = R_camera_T * T_camera_inv;
    return view_matrix;
}

Mat4f Camera::getProjectionMatrix() {
    float near = -focal_len, far = -DEFAULT_FAR;
    Mat4f persp2ortho, ortho;
    
    float aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    float top = near * tan(fov / 2 * M_PI / 180), bottom = -top;
    float right = top * aspect_ratio, left = -right;
    persp2ortho << -near, 0, 0, 0,
        0,  -near, 0, 0,
        0, 0, -(near + far), near * far,
        0, 0, -1, 0;
    ortho << - 2 / (right - left), 0, 0, -(right + left)/(right - left),
        0, -2 / (top - bottom), 0, -(top + bottom)/(top - bottom),
        0, 0, 2/ (near - far), -(near + far)/(near - far),
        0, 0, 0, 1;

    return ortho*persp2ortho;
}