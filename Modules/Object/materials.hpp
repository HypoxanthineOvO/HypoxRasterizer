#ifndef MATERIALS_HPP_
#define MATERIALS_HPP_

#include "utils.hpp"
#include <algorithm>

struct MaterialSample {
    Vec3f base_color = Vec3f::Zero();
    Vec3f normal = Vec3f(0, 0, 1);
    float shininess = 15.0f;
    float roughness = 0.5f;
    float metallic = 0.0f;
    float ao = 1.0f;
};

class Materials {
protected:
    float shininess;
    float roughness;
    float metallic;
public:
    Materials(): shininess(15.0f), roughness(0.5f), metallic(0.0f) {}
    Materials(float shininess): shininess(shininess), roughness(0.5f), metallic(0.0f) {}
    Materials(float shininess, float roughness, float metallic):
        shininess(shininess), roughness(roughness), metallic(metallic) {}
    virtual MaterialSample sample(Vec2f uv) const = 0;
    virtual Vec3f evalColor(Vec2f uv) const { return sample(uv).base_color; }
    virtual float evalShininess() const { return shininess; }
    virtual float evalRoughness() const { return roughness; }
    virtual float evalMetallic() const { return metallic; }
};

class ColorMaterial : public Materials {
    Vec3f color;
public:
    ColorMaterial() : color(0, 0, 0) {}
    ColorMaterial(Vec3f color) : color(color) {}
    ColorMaterial(Vec3f color, float shininess) : color(color), Materials(shininess) {}
    ColorMaterial(Vec3f color, float shininess, float roughness, float metallic):
        Materials(shininess, roughness, metallic), color(color) {}

    virtual MaterialSample sample(Vec2f uv) const override {
        MaterialSample result;
        result.base_color = color;
        result.shininess = shininess;
        result.roughness = roughness;
        result.metallic = metallic;
        return result;
    }
};

class PBRMaterial : public Materials {
    Vec3f base_color;
public:
    PBRMaterial(Vec3f base_color, float roughness, float metallic):
        Materials(15.0f, roughness, metallic), base_color(base_color) {}

    virtual MaterialSample sample(Vec2f uv) const override {
        MaterialSample result;
        result.base_color = base_color;
        result.shininess = shininess;
        result.roughness = roughness;
        result.metallic = metallic;
        return result;
    }
};

class TextureMaterial : public Materials {
    std::vector<Vec3f> texture;
    int width, height;
public:
    TextureMaterial(): Materials(), width(0), height(0) {}
    TextureMaterial(const std::string& file_name): 
        Materials(), width(0), height(0) {  
        loadTexture(file_name);
    }
    TextureMaterial(const std::string& file_name, float shininess) : 
        Materials(shininess), width(0), height(0) {
        loadTexture(file_name);
    }
    TextureMaterial(const std::string& file_name, float shininess, float roughness, float metallic) :
        Materials(shininess, roughness, metallic), width(0), height(0) {
        loadTexture(file_name);
    }

    virtual MaterialSample sample(Vec2f uv) const override {
        MaterialSample result;
        result.base_color = evalColor(uv);
        result.shininess = shininess;
        result.roughness = roughness;
        result.metallic = metallic;
        return result;
    }

    virtual Vec3f evalColor(Vec2f uv) const override {
        // Get the color from the texture
        if (texture.empty() || width <= 0 || height <= 0) {
            return Vec3f::Zero();
        }
        float u = std::clamp(uv.x(), 0.0f, 1.0f);
        float v = std::clamp(uv.y(), 0.0f, 1.0f);
        int x = std::min(static_cast<int>(u * width), width - 1),
            y = std::min(static_cast<int>(v * height), height - 1);
        return texture[y * width + x];
    }

    void loadTexture(const std::string& file_name) {
        // Load Texture from image file
        ImageData raw = readImageDataFromFile(file_name);
        width = raw.width;
        height = raw.height;
        // Resize the texture to width * height
        texture.resize(width * height);
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                float alpha = raw.pixels[j * width + i].w();
                texture[j * width + i] = raw.pixels[j * width + i].head(3) * alpha;
                // If not zero
                // if (raw[j*width+i].x() != 0 || raw[j*width+i].y() != 0 || raw[j*width+i].z() != 0){
                //     utils::printVec(raw[j*width+i]);
                // }
            }
        }
    }

    int getTextureWidth() const { return width; }
    int getTextureHeight() const { return height; }
};

#endif /* MATERIALS_HPP_ */
