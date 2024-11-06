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
    void initShadowMap(int res, std::vector<std::shared_ptr<Object>>& objects) {
        shadow_map = std::make_shared<ShadowMap>(res);
        shadow_map->initialize(position_proxy, Vec3f(0, -1, 0));
        shadow_map->generateDepthBuffer(objects);
    }
    bool isShadowed(Vec3f position) {
        return shadow_map->isShadowed(position);
    }
    void showShadowMap(const std::string& file_name) {
        shadow_map->showShadowMap(file_name);
    }
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
    
private:
};

// TODO: Real Light Class


#endif // LIGHT_HPP_