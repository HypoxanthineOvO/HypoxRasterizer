#include "shadowmap.hpp"
#include "image.hpp"

void ShadowMap::generateDepthBuffer(std::vector<std::shared_ptr<Object>>& objects) {
    for (std::shared_ptr<Object> obj: objects) {
        Mat4f view_mat = camera->getViewMatrix(), proj_mat = camera->getProjectionMatrix();
        for (Triangle tri: obj->getTriangles()) {
            // For each Triangle, project the vertices, and update the depth buffer
            // 1. Vertex Processing
            Triangle screen_space_tri;
            for (int i = 0; i < 3; i++) {
                Vertex vert = tri.getVertex(i);
                Vec4f org_pos = Vec4f(vert.position.x(), vert.position.y(), vert.position.z(), 1);
                
                // Apply the Transformation
                Vertex new_vert = vert;
                Vec4f pos = proj_mat * view_mat * org_pos;
                
                new_vert.position = Vec3f(pos.x() / pos.w(), pos.y() / pos.w(), pos.z() / pos.w());
                
                screen_space_tri.setVertex(i, new_vert);
            }
            
            // Transform the Triangle to Screen Space
            Vec2f min_screen = screen_space_tri.getXYMin(), max_screen = screen_space_tri.getXYMax();
            float width = static_cast<float>(resolution.x()), height = static_cast<float>(resolution.y());
            Vec2i min_screen_i = Vec2i(
                static_cast<int>((min_screen.x() + 1) / 2 * width),
                static_cast<int>((min_screen.y() + 1) / 2 * height)
            ),
            max_screen_i = Vec2i(
                static_cast<int>((max_screen.x() + 1) / 2 * width),
                static_cast<int>((max_screen.y() + 1) / 2 * height)
            );
            // Clip the bounding box
            min_screen_i = (min_screen_i - Vec2i(1, 1)).cwiseMax(Vec2i(0, 0));
            max_screen_i = (max_screen_i + Vec2i(1, 1)).cwiseMin(resolution);

            // Rasterize the Triangle
            for (int x = min_screen_i.x(); x < max_screen_i.x(); x++) {
                for (int y = min_screen_i.y(); y < max_screen_i.y(); y++) {
                    Vec3f pos = Vec3f(
                        2 * static_cast<float>(x) / width - 1,
                        2 * static_cast<float>(y) / height - 1,
                        0
                    );
                    if (screen_space_tri.isInsidefor2D(pos)) {
                        Vec3f weights = screen_space_tri.getInterpolationWeightsfor2D(pos);
                        // Check weights valid
                        if (!utils::isValidWeight(weights)) {
                            continue;
                        }
                        // Depth
                        float depth = weights.x() * screen_space_tri.getVertex(0).position.z() +
                            weights.y() * screen_space_tri.getVertex(1).position.z() +
                            weights.z() * screen_space_tri.getVertex(2).position.z();
                        // Check the Depth Buffer
                        if (std::abs((depth-1)/2) >= depth_buffer[y * resolution.x() + x]) {
                            continue;
                        }
                        // Write to the Depth Buffer
                        depth_buffer[y * resolution.x() + x] = std::abs((depth-1)/2);
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
    
    //printf("X: %d, Y: %d | ", x, y);
    if (x >= 0 && x < resolution.x() && y >= 0 && y < resolution.y()) {
        float depth = screen_pos.z();
        if (std::abs((depth-1)/2) <= depth_buffer[y * resolution.x() + x] + SHADOW_MAP_BIAS) {
            //printf("NO SHADOW! DEPTH: %f, BUFFER: %f\n", std::abs((depth-1)/2), depth_buffer[y * resolution.x() + x]);
            return false;
        }
        else {
            //printf("SHADOW! DEPTH: %f, BUFFER: %f\n", std::abs((depth-1)/2), depth_buffer[y * resolution.x() + x]);
            return true;
        }
    }
    return false;
}

void ShadowMap::showShadowMap(const std::string& file_name) {
    // Write image
    writeImageToFile(depth_buffer, resolution, file_name);
}