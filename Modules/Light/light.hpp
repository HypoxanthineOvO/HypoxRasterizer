#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "utils.hpp"
#include "shadowmap.hpp"

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
    IndirectVPL(Vec3f pos, Vec3f flx):
        position(pos), flux(flx)
    {}
    Vec3f position;
    Vec3f flux;
};

class Light {
protected:
    Vec3f position_proxy;
    std::vector<DirectVPL> direct_vpls;
    std::vector<IndirectVPL> indirect_vpls;

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

public:
    PointLight(Vec3f pos, Vec3f inten) {
        direct_vpls.clear();
        DirectVPL d_vpl(pos, inten);
        direct_vpls.push_back(d_vpl);

        position_proxy = pos;
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
        bool is_shadowed = false;
        //printf("| ");
        for (std::shared_ptr<ShadowMap> shadow_map: shadow_maps) {
            //printf("%d | ", shadow_map->isShadowed(position));
            is_shadowed = is_shadowed || shadow_map->isLighted(position);
        }
        //puts("");
        return is_shadowed;
    }

    /**
     * @brief 将点光源的阴影贴图保存为图像文件。
     * @param file_name 输出图像文件的路径。
     * @note 每个方向的阴影贴图会保存为单独的图像文件。
     */
    virtual void showShadowMap(const std::string& file_name) override {
        for (int i = 0; i < 6; i++) {
            std::string sub_file_name = file_name + "_" + std::to_string(i);
            sub_file_name = sub_file_name + "(" + 
            std::to_string(roundf(directions[i].x() * 100) / 100);
            sub_file_name = sub_file_name.substr(0, sub_file_name.size() - 4);
            sub_file_name = sub_file_name + ",";
            
            sub_file_name = sub_file_name + std::to_string(roundf(directions[i].y() * 100) / 100);
            sub_file_name = sub_file_name.substr(0, sub_file_name.size() - 4);
            sub_file_name = sub_file_name + ",";
            sub_file_name = sub_file_name + std::to_string(roundf(directions[i].z() * 100) / 100);
            sub_file_name = sub_file_name.substr(0, sub_file_name.size() - 4);
            sub_file_name = sub_file_name + ")";
            // Remove last 3 characters
            sub_file_name = sub_file_name + ".png";
            // shadow_maps[i]->showShadowMap(sub_file_name);
        }
    }
};

class AreaLight : public Light {
protected:
    std::shared_ptr<ShadowMap> shadow_map;
    Vec3f normal;
    Vec2f size;

public:
    AreaLight(Vec3f pos, Vec3f inten, Vec3f norm, Vec2f sz):
        normal(norm), size(sz)
    {
        direct_vpls.clear();
        Vec3f avg_inten = inten / (NUM_SQRT_DIRECT_VPL * NUM_SQRT_DIRECT_VPL);
        // Use normal and ref_up to get offset_right and offset_up
        Vec3f ref_up = REF_UP, ref_right = REF_RIGHT;
        Vec3f offset_up = -normal;
        Vec3f offset_right = ref_right.cross(offset_up);
        Vec3f offset_forward = offset_up.cross(offset_right);
        
        for (int i = 0; i < NUM_SQRT_DIRECT_VPL; i++) {
            for (int j = 0; j < NUM_SQRT_DIRECT_VPL; j++) {
                Vec3f d = pos + 
                    offset_right * (i - NUM_SQRT_DIRECT_VPL / 2 - 0.5) * size.x() / (2 * NUM_SQRT_DIRECT_VPL) +
                    offset_forward * (j - NUM_SQRT_DIRECT_VPL / 2 - 0.5) * size.y() / (2 * NUM_SQRT_DIRECT_VPL)
                    - 0.01 * offset_up;
                DirectVPL d_vpl(d, avg_inten);
                direct_vpls.push_back(d_vpl);
            }
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
        shadow_map->initialize(position_proxy, normal, 120);
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
        return shadow_map->isLighted(position);
    }

    /**
     * @brief 将区域光源的阴影贴图保存为图像文件。
     * @param file_name 输出图像文件的路径。
     * @note 生成的图像可视化区域光源的阴影贴图。
     */
    virtual void showShadowMap(const std::string& file_name) override {
        shadow_map->showShadowMap(file_name);
    }
};

#endif // LIGHT_HPP_