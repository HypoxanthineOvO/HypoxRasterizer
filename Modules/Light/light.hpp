#ifndef LIGHT_HPP_
#define LIGHT_HPP_

#include "utils.hpp"

class Light {
public:
    Light();
    virtual ~Light() = 0;

    virtual Vec3f getPositionProxy() = 0;
private:

};

class PointLight: public Light {
public:
    virtual Vec3f getPositionProxy() override {
        return position;
    }
private:
    Vec3f position;
    Vec3f intensity;
};

// TODO: Implement AreaLight
// class AreaLight: public Light {
// public:

// private:

// };


#endif // LIGHT_HPP_