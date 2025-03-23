#include "shadowmap.hpp"
#include "image.hpp"

/**
 * @brief 生成深度缓冲区，用于阴影计算。
 * @param objects 场景中的对象列表。
 * @note 该函数会将对象的三角形投影到屏幕空间，并更新深度缓冲区。
 *       仅支持三角形面片，且深度值范围为 [-1, 0]。
 */
void ShadowMap::generateDepthBuffer(std::vector<std::shared_ptr<Object>>& objects) {
    for (std::shared_ptr<Object> obj: objects) {
        Mat4f view_mat = camera->getViewMatrix(), proj_mat = camera->getProjectionMatrix(true);
        auto triangles = obj->getTriangles();
        for (Triangle tri: triangles) {
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
            
            // If invalid, continue
            if (
                ((min_screen.x() > 1) && (max_screen.x() > 1))
                ||
                ((min_screen.x() < -1) && (max_screen.x() < -1))
                ||
                ((min_screen.y() > 1) && (max_screen.y() > 1))
                ||
                ((min_screen.y() < -1) && (max_screen.y() < -1))
                ||
                ((min_screen.x() > max_screen.x()) || (min_screen.y() > max_screen.y()))
            ){
                continue;
            }
            
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

            // Count the pixels
            int num_pixels = (max_screen_i.x() - min_screen_i.x()) * (max_screen_i.y() - min_screen_i.y());

            if (num_pixels < 0) continue;
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
                        // Only the front-most pixel is considered
                        if (depth < -1 || depth > 0) {
                            continue;
                        }
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

/**
 * @brief 检查指定位置是否被光照到。
 * @param position 世界坐标中的位置。
 * @return 如果位置被光照到，返回 true；否则返回 false。
 * @note 该函数会将位置投影到屏幕空间，并与深度缓冲区进行比较。
 */
bool ShadowMap::isLighted(Vec3f position) const {
    Mat4f view_mat = camera->getViewMatrix(), proj_mat = camera->getProjectionMatrix(true);
    Vec4f pos = Vec4f(position.x(), position.y(), position.z(), 1);
    Vec4f proj_pos = proj_mat * view_mat * pos;
    Vec3f screen_pos = Vec3f(proj_pos.x() / proj_pos.w(), proj_pos.y() / proj_pos.w(), proj_pos.z() / proj_pos.w());
    int x = static_cast<int>((screen_pos.x() + 1) / 2 * resolution.x()),
        y = static_cast<int>((screen_pos.y() + 1) / 2 * resolution.y());
    
    if (x >= 0 && x < resolution.x() && y >= 0 && y < resolution.y()) {
        float depth = screen_pos.z();
        if (std::abs((depth-1)/2) <= depth_buffer[y * resolution.x() + x] + SHADOW_MAP_BIAS) {
            //printf("depth:%f\n",depth);
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}

/**
 * @brief 将深度缓冲区保存为图像文件。
 * @param file_name 输出图像文件的路径。
 * @note 深度值会被归一化到 [0, 1] 范围，以便可视化。
 */
void ShadowMap::showShadowMap(const std::string& file_name) {
    // Write image
    // Get min_value and max_value
    float min_value = 1, max_value = 0;
    for (int i = 0; i < camera->getWidth() * camera->getHeight(); i++) {
        min_value = std::min(min_value, depth_buffer[i]);
        max_value = std::max(max_value, depth_buffer[i]);
    }
    // Normalize the Depth Buffer
    for (int i = 0; i < camera->getWidth() * camera->getHeight(); i++) {
        depth_buffer_tofile[i] = (depth_buffer[i] - min_value) / (max_value - min_value);
    }
    writeImageToFile(depth_buffer_tofile, resolution, file_name);
}