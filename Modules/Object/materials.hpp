#ifndef MATERIALS_HPP_
#define MATERIALS_HPP_

#include "utils.hpp"

class Materials {
protected:
    float shininess;
public:
    Materials(): shininess(15.0f) {}
    Materials(float shininess): shininess(shininess) {}
    virtual Vec3f evalColor(Vec2f uv) const = 0;
    virtual float evalShininess() const { return shininess; }
};

class ColorMaterial : public Materials {
    Vec3f color;
public:
    ColorMaterial() : color(0, 0, 0) {}
    ColorMaterial(Vec3f color) : color(color) {}
    ColorMaterial(Vec3f color, float shininess) : color(color), Materials(shininess) {}

    virtual Vec3f evalColor(Vec2f uv) const override {
        return color;
    }
};

class TextureMaterial : public Materials {
    std::vector<Vec3f> texture;
    int width, height;
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

    virtual Vec3f evalColor(Vec2f uv) const override {
        // Get the color from the texture
        int x = uv.x() * width, y = uv.y() * height;
        return texture[y * width + x];
    }

    void loadTexture(const std::string& file_name) {
        // Load Texture from image file
        std::vector<Vec4f> raw = readImageFromFile(file_name);
        // Resize the texture to width * height
        texture.resize(width * height);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                float alpha = raw[j * width + i].w();
                texture[j * width + i] = raw[j * width + i].head(3) * alpha;
                // If not zero
                // if (raw[j*width+i].x() != 0 || raw[j*width+i].y() != 0 || raw[j*width+i].z() != 0){
                //     utils::printVec(raw[j*width+i]);
                // }
            }
        }
    }
};

#endif /* MATERIALS_HPP_ */