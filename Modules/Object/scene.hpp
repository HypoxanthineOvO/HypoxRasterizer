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
    void addObject(std::shared_ptr<Object> obj) { objects.push_back(obj); }
    void addLight(std::shared_ptr<Light> light) { lights.push_back(light); }
    /* Getters */
    std::vector<std::shared_ptr<Object>> getObjects() { return objects; }
    std::vector<std::shared_ptr<Light>> getLights() { return lights; }
private:
    std::vector<std::shared_ptr<Light>> lights;
    std::vector<std::shared_ptr<Object>> objects;
};

#endif // SCENE_HPP_