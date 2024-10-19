#ifndef SCENE_HPP_
#define SCENE_HPP_

#include "object.hpp"
#include "light.hpp"

class Scene {
public:
    Scene() {
        objects.clear();
        lights.clear();
    }
    /* Modify Functions */
    void addObject(Object obj) { objects.push_back(obj); }
    void addLight(Light light) { lights.push_back(light); }

    /* Getters */
    std::vector<Object> getObjects() { return objects; }
    std::vector<Light> getLights() { return lights; }
private:
    std::vector<Light> lights;
    std::vector<Object> objects;
};

#endif // SCENE_HPP_