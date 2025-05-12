#include "rsm.hpp"
#include <algorithm>
#include <numeric>

void RSM::generateRSMBuffer(std::vector<std::shared_ptr<Object>>& objects, float lightIntensity) {
    // 1. 调用基类方法生成深度图
    generateDepthBuffer(objects);

    // 2. 清空RSM特有缓冲区
    std::fill(position_buffer.begin(), position_buffer.end(), Vec3f::Zero());
    std::fill(normal_buffer.begin(), normal_buffer.end(), Vec3f::Zero());
    std::fill(flux_buffer.begin(), flux_buffer.end(), 0.0f);

    // 3. 获取变换矩阵
    Mat4f view_mat = camera->getViewMatrix();
    Mat4f proj_mat = camera->getProjectionMatrix(true);
    Mat4f view_proj_mat = proj_mat * view_mat;
    Mat4f inv_view_mat = view_mat.inverse();

    // 4. 处理每个物体
    for (const auto& obj : objects) {
        //material中需要统一一下材质的反射率
        //float reflectivity = obj->getMaterial()->getReflectivity();
        //Vec3f albedo = obj->getMaterial()->getAlbedo();
        
        for (const Triangle& tri : obj->getTriangles()) {
            // 5. 顶点处理（与ShadowMap一致）
            Triangle screen_space_tri;
            for (int i = 0; i < 3; ++i) {
                Vertex vert = tri.getVertex(i);
                Vec4f org_pos = Vec4f(vert.position.x(), vert.position.y(), vert.position.z(), 1.0f);
                Vec4f pos = view_proj_mat * org_pos;
                
                Vertex new_vert = vert;
                new_vert.position = Vec3f(pos.x() / pos.w(), pos.y() / pos.w(), pos.z() / pos.w());
                screen_space_tri.setVertex(i, new_vert);
            }

            // 6. 计算屏幕空间AABB
            Vec2f min_screen = screen_space_tri.getXYMin();
            Vec2f max_screen = screen_space_tri.getXYMax();
            
            // 7. 裁剪检查（与ShadowMap一致）
            if (((min_screen.x() > 1.0f) && (max_screen.x() > 1.0f)) ||
                ((min_screen.x() < -1.0f) && (max_screen.x() < -1.0f)) ||
                ((min_screen.y() > 1.0f) && (max_screen.y() > 1.0f)) ||
                ((min_screen.y() < -1.0f) && (max_screen.y() < -1.0f))) {
                continue;
            }

            // 8. 计算像素范围
            float width = static_cast<float>(resolution.x());
            float height = static_cast<float>(resolution.y());
            Vec2i min_screen_i = Vec2i(
                static_cast<int>((min_screen.x() + 1.0f) * 0.5f * width),
                static_cast<int>((min_screen.y() + 1.0f) * 0.5f * height));
            Vec2i max_screen_i = Vec2i(
                static_cast<int>((max_screen.x() + 1.0f) * 0.5f * width),
                static_cast<int>((max_screen.y() + 1.0f) * 0.5f * height));
            
            // 9. 边界裁剪
            min_screen_i = (min_screen_i - Vec2i(1, 1)).cwiseMax(Vec2i(0, 0));
            max_screen_i = (max_screen_i + Vec2i(1, 1)).cwiseMin(resolution);

            // 10. 光栅化三角形
            for (int y = min_screen_i.y(); y < max_screen_i.y(); ++y) {
                for (int x = min_screen_i.x(); x < max_screen_i.x(); ++x) {
                    Vec3f pixel_pos = Vec3f(
                        2.0f * static_cast<float>(x) / width - 1.0f,
                        2.0f * static_cast<float>(y) / height - 1.0f,
                        0.0f);
                    
                    if (screen_space_tri.isInsidefor2D(pixel_pos)) {
                        // 11. 计算插值权重
                        Vec3f weights = screen_space_tri.getInterpolationWeightsfor2D(pixel_pos);
                        if (!utils::isValidWeight(weights)) {
                            continue;
                        }

                        // 12. 深度测试（与ShadowMap一致）
                        float depth = weights.x() * screen_space_tri.getVertex(0).position.z() +
                                     weights.y() * screen_space_tri.getVertex(1).position.z() +
                                     weights.z() * screen_space_tri.getVertex(2).position.z();
                        
                        if (depth < -1.0f || depth > 0.0f) continue;
                        if (std::abs((depth - 1.0f) / 2.0f) >= depth_buffer[y * resolution.x() + x]) continue;

                        // 13. 计算世界坐标
                        Vec4f screen_pos(pixel_pos.x(), pixel_pos.y(), depth, 1.0f);
                        Vec4f view_pos = proj_mat.inverse() * screen_pos;
                        view_pos /= view_pos.w();
                        Vec4f world_pos = inv_view_mat * view_pos;

                        // 14. 计算法线（世界空间）
                        Vec3f normal = weights.x() * tri.getVertex(0).normal +
                                      weights.y() * tri.getVertex(1).normal +
                                      weights.z() * tri.getVertex(2).normal;
                        normal.normalize();

                        // 15. 计算通量（论文公式实现）
                        Vec3f light_dir = (camera->getPosition() - Vec3f(world_pos.x(), world_pos.y(), world_pos.z())).normalized();
                        float cos_theta = std::max(0.0f, normal.dot(-light_dir));
                        float flux = lightIntensity * cos_theta;

                        // 16. 存储数据
                        int idx = y * resolution.x() + x;
                        position_buffer[idx] = Vec3f(world_pos.x(), world_pos.y(), world_pos.z());
                        normal_buffer[idx] = normal;
                        flux_buffer[idx] = flux;
                    }
                }
            }
        }
    }
}

std::vector<VPL> RSM::generateVPLs(int targetCount) const {
    std::vector<VPL> vpls;
    if (targetCount <= 0 || flux_buffer.empty()) return vpls;

    // 1. 使用改进的空间均匀性采样获取索引
    std::vector<int> sampledIndices = importanceSampling(targetCount);

    // 2. 计算总通量（仅计算采样点的通量）
    float total_flux = 0.0f;
    for (int idx : sampledIndices) {
        total_flux += flux_buffer[idx];
    }
    if (total_flux <= 0.0f) return vpls;

    // 3. 生成VPL（带法线偏移和通量归一化）
    const float NORMAL_OFFSET = 0.01f;
    vpls.reserve(sampledIndices.size());
    
    for (int idx : sampledIndices) {
        // 获取采样点数据
        Vec3f pos = position_buffer[idx];
        Vec3f normal = normal_buffer[idx];
        float flux = flux_buffer[idx];

        // 应用法线偏移和通量归一化
        Vec3f offsetPos = pos + normal * NORMAL_OFFSET;
        Vec3f flx = Vec3f::Ones() * (flux / total_flux);
        
        vpls.emplace_back(offsetPos, flx, normal);
    }

    return vpls;
}

std::vector<int> RSM::importanceSampling(int targetCount) const {
    std::vector<int> sampledIndices;
    sampledIndices.reserve(targetCount);

    const int centerX = resolution.x() / 2;
    const int centerY = resolution.y() / 2;
    const int centerIdx = centerY * resolution.x() + centerX;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> distR(1, 64);  // 半径范围
    std::uniform_int_distribution<int> distTheta(0, 63); // 角度细分

    // 螺旋式采样
    while (sampledIndices.size() < targetCount) {
        int r = distR(gen);
        int theta = distTheta(gen);
        
        float angle = theta * (2.0f * M_PI / 64.0f);
        float sinA = sin(angle);
        float cosA = cos(angle);
        
        // 四个象限对称采样
        for (int quadrant = 0; quadrant < 4 && sampledIndices.size() < targetCount; ++quadrant) {
            float qAngle = angle + quadrant * (M_PI / 2.0f);
            int dx = static_cast<int>(r * cos(qAngle));
            int dy = static_cast<int>(r * sin(qAngle));
            
            int x = centerX + dx;
            int y = centerY + dy;
            
            if (x >= 0 && x < resolution.x() && y >= 0 && y < resolution.y()) {
                int idx = y * resolution.x() + x;
                if (flux_buffer[idx] > 0.001f) {
                    sampledIndices.push_back(idx);
                }
            }
        }
    }

    // 后备随机采样（如果螺旋采样不足）
    if (sampledIndices.size() < targetCount) {
        std::vector<int> validIndices;
        for (int i = 0; i < flux_buffer.size(); ++i) {
            if (flux_buffer[i] > 0.001f) {
                validIndices.push_back(i);
            }
        }
        
        std::shuffle(validIndices.begin(), validIndices.end(), gen);
        while (sampledIndices.size() < targetCount && !validIndices.empty()) {
            sampledIndices.push_back(validIndices.back());
            validIndices.pop_back();
        }
    }

    return sampledIndices;
}

void RSM::showRSM(const std::string& file_name) {
    // 1. 准备输出数据
    std::vector<Vec3f> pos_out(position_buffer.size());
    std::vector<Vec3f> norm_out(normal_buffer.size());
    std::vector<Vec3f> flux_out(flux_buffer.size());

    // 2. 归一化处理
    float max_flux = *std::max_element(flux_buffer.begin(), flux_buffer.end());
    
    for (size_t i = 0; i < position_buffer.size(); ++i) {
        // 位置和法线归一化到[0,1]
        pos_out[i] = (position_buffer[i] + Vec3f(1.0f, 1.0f, 1.0f)) * 0.5f;
        norm_out[i] = (normal_buffer[i] + Vec3f(1.0f, 1.0f, 1.0f)) * 0.5f;
        
        // 通量归一化
        float flux_val = max_flux > 0.0f ? flux_buffer[i] / max_flux : 0.0f;
        flux_out[i] = Vec3f(flux_val, flux_val, flux_val);
    }

    // 3. 写入文件
    writeImageToFile(pos_out, resolution, file_name + "_pos.png");
    writeImageToFile(norm_out, resolution, file_name + "_norm.png");
    writeImageToFile(flux_out, resolution, file_name + "_flux.png");
}