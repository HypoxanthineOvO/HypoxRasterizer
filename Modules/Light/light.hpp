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
    std::shared_ptr<ShadowMap> shadow_map;
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
            std::to_string(roundf(directions[i].x() * 100) / 100)
            + "," + 
            std::to_string(roundf(directions[i].y() * 100) / 100)
            + "," + 
            std::to_string(roundf(directions[i].z() * 100) / 100)
            + ")";
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

// TODO: Real Light Class


#endif // LIGHT_HPP_11