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
    TextureMaterial(): Materials(shininess), width(1024), height(1024) {}
    TextureMaterial(const std::string& file_name): 
        Materials(shininess), width(1024), height(1024) {  
        loadTexture(file_name);
    }
    TextureMaterial(const std::string& file_name, float shininess) : 
        Materials(shininess), width(1024), height(1024) {
        loadTexture(file_name);
    }

    virtual Vec3f evalColor(Vec2f uv) override {
        // Get the color from the texture
        int x = uv.x() * width, y = uv.y() * height;
        printf("x: %d, y: %d\n", x, y);
        return texture[y * width + x];
    }

    void loadTexture(const std::string& file_name) {
        // Load Texture from image file
        std::vector<Vec3f> raw = readImageFromFile(file_name);
        // Resize the texture to width * height
        texture.resize(width * height);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                texture[j * width + i] = raw[j * width + i];
            }
        }
    }
private:
    std::vector<Vec3f> texture;
    int width, height;
};

#endif /* MATERIALS_HPP_ */