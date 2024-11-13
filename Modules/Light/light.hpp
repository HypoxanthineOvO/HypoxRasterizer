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
public:
    Light() {}
    Light(std::vector<DirectVPL> d_vpls, std::vector<IndirectVPL> i_vpls):
        direct_vpls(d_vpls), indirect_vpls(i_vpls)
    {}
    Vec3f getPosition() {
        return position_proxy;
    }
    std::vector<DirectVPL> getDirectVPLs() {
        return direct_vpls;
    }
    std::vector<IndirectVPL> getIndirectVPLs() {
        return indirect_vpls;
    }
    virtual void initShadowMap(int res, std::vector<std::shared_ptr<Object>>& objects) = 0;
    virtual bool isShadowed(Vec3f position) = 0;
    virtual void showShadowMap(const std::string& file_name) = 0;
protected:
    Vec3f position_proxy;
    std::vector<DirectVPL> direct_vpls;
    std::vector<IndirectVPL> indirect_vpls;
};

class PointLight: public Light {
public:
    PointLight(Vec3f pos, Vec3f inten) {
        direct_vpls.clear();
        DirectVPL d_vpl(pos, inten);
        direct_vpls.push_back(d_vpl);

        position_proxy = pos;
    }

    virtual void initShadowMap(int res, std::vector<std::shared_ptr<Object>>& objects) override {


        shadow_maps.clear();
        for (int i = 0; i < 6; i++) {
            std::shared_ptr<ShadowMap> shadow_map = std::make_shared<ShadowMap>(res);
            shadow_map->initialize(position_proxy, directions[i]);
            shadow_map->generateDepthBuffer(objects);
            shadow_maps.push_back(shadow_map);
        }
    }
    
    virtual bool isShadowed(Vec3f position) override {
        bool is_shadowed = false;
        //printf("| ");
        for (std::shared_ptr<ShadowMap> shadow_map: shadow_maps) {
            //printf("%d | ", shadow_map->isShadowed(position));
            is_shadowed = is_shadowed || shadow_map->isShadowed(position);
        }
        //puts("");
        return is_shadowed;
    }

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
            shadow_maps[i]->showShadowMap(sub_file_name);
        }
    }

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
};

class AreaLight: public Light {
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

    virtual void initShadowMap(int res, std::vector<std::shared_ptr<Object>>& objects) override {
        std::shared_ptr<ShadowMap> shadow_map = std::make_shared<ShadowMap>(res);
        shadow_map->initialize(position_proxy, normal, 53);
        // printf("Generating Depth Buffer\n");
        // printf("POSITON: %f %f %f\n", position_proxy.x(), position_proxy.y(), position_proxy.z());
        // printf("NORMAL: %f %f %f\n", normal.x(), normal.y(), normal.z());
        shadow_map->generateDepthBuffer(objects);

        this->shadow_map = shadow_map;
    }

    virtual bool isShadowed(Vec3f position) override {
        return shadow_map->isShadowed(position);
    }

    virtual void showShadowMap(const std::string& file_name) override {
        shadow_map->showShadowMap(file_name);
    }

protected:
    std::shared_ptr<ShadowMap> shadow_map;
    Vec3f normal;
    Vec2f size;
};

#endif // LIGHT_HPP_11