#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "utils.hpp"
#include "shadowmap.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace light_detail {

inline Vec3f normalizedOrDefault(Vec3f value, const Vec3f& fallback) {
    if (value.norm() < LENGTH_EPS) {
        return fallback.normalized();
    }
    return value.normalized();
}

inline std::string shadowOutputStem(const std::string& file_name) {
    const std::string suffix = ".png";
    if (file_name.size() >= suffix.size() &&
        file_name.substr(file_name.size() - suffix.size()) == suffix) {
        return file_name.substr(0, file_name.size() - suffix.size());
    }
    return file_name;
}

}  // namespace light_detail

class DirectVPL {
public:
    DirectVPL(Vec3f pos, Vec3f inten):
        position(pos), intensity(inten)
    {}
    Vec3f position;
    Vec3f intensity;
};

class IndirectVPL {
public:
    IndirectVPL(
        Vec3f pos,
        Vec3f norm,
        Vec3f flx,
        float sample_radius = 1.0f,
        int bounce = 1,
        int source_light = 0,
        uint32_t sample_flags = 0
    ):
        position(pos),
        normal(light_detail::normalizedOrDefault(norm, Vec3f(0, 1, 0))),
        flux(flx),
        radius(sample_radius),
        bounce_index(bounce),
        source_light_index(source_light),
        flags(sample_flags)
    {}
    Vec3f position;
    Vec3f normal;
    Vec3f flux;
    float radius = 1.0f;
    int bounce_index = 1;
    int source_light_index = 0;
    uint32_t flags = 0;

    std::vector<float> toFloatRecord() const {
        return {
            position.x(), position.y(), position.z(),
            normal.x(), normal.y(), normal.z(),
            flux.x(), flux.y(), flux.z(),
            radius,
            static_cast<float>(bounce_index),
            static_cast<float>(source_light_index),
            static_cast<float>(flags)
        };
    }
};

class Light {
protected:
    Vec3f position_proxy;
    std::vector<DirectVPL> direct_vpls;
    std::vector<IndirectVPL> indirect_vpls;
    bool indirect_vpl_visibility = true;
    float indirect_vpl_clamp = 10.0f;

public:
    Light() {}
    Light(std::vector<DirectVPL> d_vpls, std::vector<IndirectVPL> i_vpls):
        direct_vpls(d_vpls), indirect_vpls(i_vpls)
    {}
    Vec3f getPosition() const {
        return position_proxy;
    }
    std::vector<DirectVPL> getDirectVPLs() const {
        return direct_vpls;
    }
    std::vector<IndirectVPL> getIndirectVPLs() const {
        return indirect_vpls;
    }
    void setIndirectVPLs(
        const std::vector<IndirectVPL>& samples,
        bool visibility = true,
        float clamp = 10.0f
    ) {
        indirect_vpls = samples;
        indirect_vpl_visibility = visibility;
        indirect_vpl_clamp = clamp;
    }
    bool isIndirectVPLVisibilityEnabled() const {
        return indirect_vpl_visibility;
    }
    float getIndirectVPLClamp() const {
        return indirect_vpl_clamp;
    }

    /**
     * @brief 初始化阴影贴图。
     * @param res 阴影贴图的分辨率。
     * @param objects 场景中的对象列表。
     * @note 该函数会为光源生成阴影贴图，用于阴影计算。
     */
    virtual void initShadowMap(int res, std::vector<std::shared_ptr<Object>>& objects) = 0;

    /**
     * @brief 检查指定位置是否被光照到。
     * @param position 世界坐标中的位置。
     * @return 如果位置被光照到，返回 true；否则返回 false。
     * @note 该函数会使用阴影贴图判断位置是否在阴影中。
     */
    virtual bool isLighted(Vec3f position) const = 0;

    /**
     * @brief 将阴影贴图保存为图像文件。
     * @param file_name 输出图像文件的路径。
     * @note 该函数用于调试，生成的图像可视化阴影贴图。
     */
    virtual void showShadowMap(const std::string& file_name) = 0;
};

class PointLight: public Light {
protected:
    std::vector<std::shared_ptr<ShadowMap>> shadow_maps;

    // Directions Constants
    Vec3f directions[6] = {
        Vec3f(1, 0, 0),
        Vec3f(-1, 0, 0),
        Vec3f(0, 1, 0),
        Vec3f(0, -1, 0),
        Vec3f(0, 0, 1),
        Vec3f(0, 0, -1)
    };
    std::array<std::string, 6> face_labels = {"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
    std::array<std::string, 6> face_file_labels = {"pos_x", "neg_x", "pos_y", "neg_y", "pos_z", "neg_z"};

public:
    PointLight(Vec3f pos, Vec3f inten) {
        direct_vpls.clear();
        DirectVPL d_vpl(pos, inten);
        direct_vpls.push_back(d_vpl);

        position_proxy = pos;
    }

    std::vector<std::string> getCubeShadowFaceLabels() const {
        return std::vector<std::string>(face_labels.begin(), face_labels.end());
    }

    /**
     * @brief 初始化点光源的阴影贴图。
     * @param res 阴影贴图的分辨率。
     * @param objects 场景中的对象列表。
     * @note 点光源需要为每个方向生成 6 张阴影贴图。
     */
    virtual void initShadowMap(int res, std::vector<std::shared_ptr<Object>>& objects) override {
        shadow_maps.clear();
        for (int i = 0; i < 6; i++) {
            std::shared_ptr<ShadowMap> shadow_map = std::make_shared<ShadowMap>(res);
            shadow_map->initialize(position_proxy, directions[i]);
            shadow_map->generateDepthBuffer(objects);
            shadow_maps.push_back(shadow_map);
        }
    }
    
    /**
     * @brief 检查指定位置是否被点光源照亮。
     * @param position 世界坐标中的位置。
     * @return 如果位置被光照到，返回 true；否则返回 false。
     * @note 该函数会遍历所有方向的阴影贴图进行判断。
     */
    virtual bool isLighted(Vec3f position) const override {
        if (shadow_maps.empty()) {
            return true;
        }
        bool is_lighted = false;
        //printf("| ");
        for (std::shared_ptr<ShadowMap> shadow_map: shadow_maps) {
            //printf("%d | ", shadow_map->isShadowed(position));
            is_lighted = is_lighted || shadow_map->isLighted(position);
        }
        //puts("");
        return is_lighted;
    }

    /**
     * @brief 将点光源的阴影贴图保存为图像文件。
     * @param file_name 输出图像文件的路径。
     * @note 每个方向的阴影贴图会保存为单独的图像文件。
     */
    virtual void showShadowMap(const std::string& file_name) override {
        const std::string stem = light_detail::shadowOutputStem(file_name);
        for (int i = 0; i < static_cast<int>(shadow_maps.size()); i++) {
            std::string sub_file_name = stem + "_face_" + face_file_labels[i] + ".png";
            shadow_maps[i]->showShadowMap(sub_file_name);
        }
    }
};

class DirectionalLight: public Light {
protected:
    std::shared_ptr<ShadowMap> shadow_map;
    Vec3f direction;
    Vec3f intensity;

public:
    DirectionalLight(Vec3f dir, Vec3f inten):
        direction(light_detail::normalizedOrDefault(dir, Vec3f(0, -1, 0))),
        intensity(inten)
    {
        direct_vpls.clear();
        position_proxy = -direction * (DEFAULT_FAR * 0.5f);
        direct_vpls.push_back(DirectVPL(position_proxy, intensity));
    }

    Vec3f getDirection() const {
        return direction;
    }

    virtual void initShadowMap(int res, std::vector<std::shared_ptr<Object>>& objects) override {
        shadow_map = std::make_shared<ShadowMap>(res);
        shadow_map->initialize(position_proxy, direction, DEFAULT_FOV);
        shadow_map->generateDepthBuffer(objects);
    }

    virtual bool isLighted(Vec3f position) const override {
        if (shadow_map == nullptr) {
            return true;
        }
        return shadow_map->isLighted(position);
    }

    virtual void showShadowMap(const std::string& file_name) override {
        if (shadow_map == nullptr) {
            return;
        }
        shadow_map->showShadowMap(light_detail::shadowOutputStem(file_name) + ".png");
    }
};

class AreaLight : public Light {
protected:
    std::shared_ptr<ShadowMap> shadow_map;
    Vec3f normal;
    Vec2f size;
    int vpl_sample_count;
    uint32_t vpl_seed;
    bool vpl_stochastic;

public:
    AreaLight(
        Vec3f pos,
        Vec3f inten,
        Vec3f norm,
        Vec2f sz,
        int sample_count = NUM_SQRT_DIRECT_VPL * NUM_SQRT_DIRECT_VPL,
        uint32_t seed = 0,
        bool stochastic = false
    ):
        normal(light_detail::normalizedOrDefault(norm, Vec3f(0, -1, 0))),
        size(sz),
        vpl_sample_count(std::max(1, sample_count)),
        vpl_seed(seed),
        vpl_stochastic(stochastic)
    {
        direct_vpls.clear();
        Vec3f tangent = normal.cross(REF_UP);
        if (tangent.norm() < LENGTH_EPS) {
            tangent = normal.cross(REF_RIGHT);
        }
        tangent = tangent.normalized();
        Vec3f bitangent = normal.cross(tangent).normalized();

        const int columns = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(vpl_sample_count))));
        const int rows = static_cast<int>(std::ceil(static_cast<float>(vpl_sample_count) / static_cast<float>(columns)));
        Vec3f avg_inten = inten / static_cast<float>(vpl_sample_count);

        std::mt19937 rng(vpl_seed);
        std::uniform_real_distribution<float> jitter(0.0f, 1.0f);
        for (int idx = 0; idx < vpl_sample_count; ++idx) {
            const int row = idx / columns;
            const int column = idx % columns;
            const float sample_x = vpl_stochastic ? jitter(rng) : 0.5f;
            const float sample_y = vpl_stochastic ? jitter(rng) : 0.5f;
            const float u = (static_cast<float>(column) + sample_x) / static_cast<float>(columns) - 0.5f;
            const float v = (static_cast<float>(row) + sample_y) / static_cast<float>(rows) - 0.5f;
            Vec3f d = pos + tangent * (u * size.x()) + bitangent * (v * size.y());
            direct_vpls.push_back(DirectVPL(d, avg_inten));
        }


        position_proxy = pos;
    }

    /**
     * @brief 初始化区域光源的阴影贴图。
     * @param res 阴影贴图的分辨率。
     * @param objects 场景中的对象列表。
     * @note 区域光源只需要生成一张阴影贴图。
     */
    virtual void initShadowMap(int res, std::vector<std::shared_ptr<Object>>& objects) override {
        std::shared_ptr<ShadowMap> shadow_map = std::make_shared<ShadowMap>(res);
        shadow_map->initialize(position_proxy, normal, DEFAULT_FOV);
        // printf("Generating Depth Buffer\n");
        // printf("POSITON: %f %f %f\n", position_proxy.x(), position_proxy.y(), position_proxy.z());
        // printf("NORMAL: %f %f %f\n", normal.x(), normal.y(), normal.z());
        shadow_map->generateDepthBuffer(objects);

        this->shadow_map = shadow_map;
    }

    /**
     * @brief 检查指定位置是否被区域光源照亮。
     * @param position 世界坐标中的位置。
     * @return 如果位置被光照到，返回 true；否则返回 false。
     * @note 该函数会使用区域光源的阴影贴图进行判断。
     */
    virtual bool isLighted(Vec3f position) const override {
        if (shadow_map == nullptr) {
            return true;
        }
        return shadow_map->isLighted(position);
    }

    /**
     * @brief 将区域光源的阴影贴图保存为图像文件。
     * @param file_name 输出图像文件的路径。
     * @note 生成的图像可视化区域光源的阴影贴图。
     */
    virtual void showShadowMap(const std::string& file_name) override {
        if (shadow_map == nullptr) {
            return;
        }
        shadow_map->showShadowMap(file_name);
    }
};

#endif // LIGHT_HPP_
