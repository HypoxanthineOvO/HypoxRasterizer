#ifndef RSM_HPP_
#define RSM_HPP_

#include "shadowmap.hpp"
#include "image.hpp"
#include <vector>
#include <random>

struct VPL {
    Vec3f position;     // 光源位置（带法线偏移）
    Vec3f flux;    // 光强（RGB）
    Vec3f normal;       // 原始表面法线
    
    VPL(Vec3f pos, Vec3f flux, Vec3f norm) 
        : position(pos), flux(flux), normal(norm) {}
};

class RSM : public ShadowMap {
public:
    // 构造函数（完全匹配ShadowMap）
    RSM() : ShadowMap(DEFAULT_RSM_RESOLUTION), 
           position_buffer(resolution.x() * resolution.y(), Vec3f::Zero()),
           normal_buffer(resolution.x() * resolution.y(), Vec3f::Zero()),
           flux_buffer(resolution.x() * resolution.y(), 0.0f) {}
    
    RSM(int res) : ShadowMap(res),
                 position_buffer(resolution.x() * resolution.y(), Vec3f::Zero()),
                 normal_buffer(resolution.x() * resolution.y(), Vec3f::Zero()),
                 flux_buffer(resolution.x() * resolution.y(), 0.0f) {}
    
    RSM(int x, int y) : ShadowMap(x, y),
                      position_buffer(resolution.x() * resolution.y(), Vec3f::Zero()),
                      normal_buffer(resolution.x() * resolution.y(), Vec3f::Zero()),
                      flux_buffer(resolution.x() * resolution.y(), 0.0f) {}
    
    RSM(Vec2i res) : ShadowMap(res),
                    position_buffer(resolution.x() * resolution.y(), Vec3f::Zero()),
                    normal_buffer(resolution.x() * resolution.y(), Vec3f::Zero()),
                    flux_buffer(resolution.x() * resolution.y(), 0.0f) {}

    // 初始化方法（参数与ShadowMap完全一致）
    void initialize(Vec3f position, Vec3f direction, float fov = 90) {
        ShadowMap::initialize(position, direction, fov);
    }

    /**
     * @brief 生成RSM缓冲区
     * @param objects 场景对象列表
     * @param lightIntensity 光源总强度（标量）
     */
    void generateRSMBuffer(std::vector<std::shared_ptr<Object>>& objects, float lightIntensity);

    /**
     * @brief 生成虚拟点光源集合
     * @param targetCount 需要生成的VPL数量
     * @return 包含位置、光强和法线的VPL列表
     */
    std::vector<VPL> generateVPLs(int targetCount) const;

    // 缓冲区访问
    const std::vector<Vec3f>& getPositionBuffer() const { return position_buffer; }
    const std::vector<Vec3f>& getNormalBuffer() const { return normal_buffer; }
    const std::vector<float>& getFluxBuffer() const { return flux_buffer; }

    /**
     * @brief 保存RSM缓冲区到图像文件
     * @param file_name 输出文件基础路径
     */
    void showRSM(const std::string& file_name);

private:
    // 数据缓冲区
    std::vector<Vec3f> position_buffer;  // 世界坐标
    std::vector<Vec3f> normal_buffer;    // 法线向量
    std::vector<float> flux_buffer;      // 辐射通量（标量）

    // 重要性采样实现
    void importanceSampling(int targetCount,
                          std::vector<Vec3f>& outPositions,
                          std::vector<Vec3f>& outNormals,
                          std::vector<float>& outFluxes) const;
};

#endif // RSM_HPP_