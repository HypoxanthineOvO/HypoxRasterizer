#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "utils.hpp"

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
    std::vector<DirectVPL> getDirectVPLs() {
        return direct_vpls;
    }
    std::vector<IndirectVPL> getIndirectVPLs() {
        return indirect_vpls;
    }
protected:
    std::vector<DirectVPL> direct_vpls;
    std::vector<IndirectVPL> indirect_vpls;
};

class PointLight: public Light {
public:
    PointLight(Vec3f pos, Vec3f inten) {
        direct_vpls.clear();
        DirectVPL d_vpl(pos, inten);
        direct_vpls.push_back(d_vpl);
    }
    
private:
};

// TODO: Real Light Class


#endif // LIGHT_HPP_