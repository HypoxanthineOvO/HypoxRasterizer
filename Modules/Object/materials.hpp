#ifndef MATERIALS_HPP_
#define MATERIALS_HPP_

#include "utils.hpp"

class Materials {
public:
    Materials() {}
    virtual Vec3f evalColor(Vec2f uv) = 0;
private:

};

class ColorMaterial : public Materials {
public:
    ColorMaterial() : color(0, 0, 0) {}
    ColorMaterial(Vec3f color) : color(color) {}

    virtual Vec3f evalColor(Vec2f uv) override {
        return color;
    }
private:
    Vec3f color;
};

class TextureMaterial : public Materials {
public:

private:
};

#endif /* MATERIALS_HPP_ */