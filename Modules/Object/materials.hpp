#ifndef MATERIALS_HPP_
#define MATERIALS_HPP_

#include "utils.hpp"

class Materials {
public:
    Materials(): shininess(15.0f) {}
    Materials(float shininess): shininess(shininess) {}
    virtual Vec3f evalColor(Vec2f uv) = 0;
    virtual float evalShininess() { return shininess; }
protected:
    float shininess;
};

class ColorMaterial : public Materials {
public:
    ColorMaterial() : color(0, 0, 0) {}
    ColorMaterial(Vec3f color) : color(color) {}
    ColorMaterial(Vec3f color, float shininess) : color(color), Materials(shininess) {}

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