#include "shadowmap.hpp"

void ShadowMap::generateDepthBuffer(std::vector<std::shared_ptr<Object>>& objects) {
    for (std::shared_ptr<Object> obj: objects) {
        Mat4f view_mat = camera->getViewMatrix(), proj_mat = camera->getProjectionMatrix();
        for (Triangle tri: obj->getTriangles()) {
            for (int i = 0; i < 3; i++) {
                Vec4f pos = Vec4f(tri.getVertex(i).position.x(), tri.getVertex(i).position.y(), tri.getVertex(i).position.z(), 1);
                Vec4f proj_pos = proj_mat * view_mat * pos;
                Vec3f screen_pos = Vec3f(proj_pos.x() / proj_pos.w(), proj_pos.y() / proj_pos.w(), proj_pos.z() / proj_pos.w());
                int x = static_cast<int>((screen_pos.x() + 1) / 2 * resolution.x()),
                    y = static_cast<int>((screen_pos.y() + 1) / 2 * resolution.y());
                if (x >= 0 && x < resolution.x() && y >= 0 && y < resolution.y()) {
                    float depth = screen_pos.z();
                    float depth_normalized = std::abs((depth - 1) / 2);
                    if (depth_normalized < depth_buffer[y * resolution.x() + x]) {
                        depth_buffer[y * resolution.x() + x] = depth_normalized;
                    }
                }
            }
        }
    }
}

bool ShadowMap::isShadowed(Vec3f position) {
    Mat4f view_mat = camera->getViewMatrix(), proj_mat = camera->getProjectionMatrix();
    Vec4f pos = Vec4f(position.x(), position.y(), position.z(), 1);
    Vec4f proj_pos = proj_mat * view_mat * pos;
    Vec3f screen_pos = Vec3f(proj_pos.x() / proj_pos.w(), proj_pos.y() / proj_pos.w(), proj_pos.z() / proj_pos.w());
    int x = static_cast<int>((screen_pos.x() + 1) / 2 * resolution.x()),
        y = static_cast<int>((screen_pos.y() + 1) / 2 * resolution.y());
    if (x >= 0 && x < resolution.x() && y >= 0 && y < resolution.y()) {
        float depth = screen_pos.z();
        float depth_normalized = std::abs((depth - 1) / 2);
        if (depth_normalized < depth_buffer[y * resolution.x() + x]) {
            return true;
        }
    }
    return false;
}