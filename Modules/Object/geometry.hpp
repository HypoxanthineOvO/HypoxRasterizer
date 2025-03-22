#ifndef GEOMETRY_HPP_
#define GEOMETRY_HPP_

#include "utils.hpp"
#include "materials.hpp"

class Vertex {
public:
    /* Constructors */
    Vertex() = delete; // No Valid Default Constructor for Vertex
    Vertex(Vec3f pos) : position(pos), normal(1, 0, 0), uv(0, 0){}
    Vertex(float x, float y, float z) : position(x, y, z), normal(1, 0, 0), uv(0, 0){}
    Vertex(Vec3f pos, Vec3f norm) : position(pos), normal(norm), uv(0, 0){}
    Vertex(Vec3f pos, Vec3f norm, Vec2f uv) : position(pos), normal(norm), uv(uv){}

    Vec3f position;
    Vec3f normal;
    Vec2f uv;
};

class Triangle {
    Vertex v0, v1, v2;
    std::shared_ptr<Materials> material;
public:
    /* Constructors */
    Triangle(): v0(Vec3f(0, 0, 0)), v1(Vec3f(0, 0, 0)), v2(Vec3f(0, 0, 0)), material(nullptr) {}
    Triangle(Vertex v0, Vertex v1, Vertex v2) : v0(v0), v1(v1), v2(v2), material(nullptr) {}
    Triangle(Vertex v0, Vertex v1, Vertex v2, std::shared_ptr<Materials> mat) : v0(v0), v1(v1), v2(v2), material(mat) {}

    /* Getters and Setters */
    Vertex getVertex(int idx) const {
        if (idx == 0) {
            return v0;
        } else if (idx == 1) {
            return v1;
        } else if (idx == 2) {
            return v2;
        } else {
            throw std::runtime_error("Invalid Vertex Index");
        }
    }
    void setVertex(int idx, Vertex vert) {
        if (idx == 0) {
            v0 = vert;
        } else if (idx == 1) {
            v1 = vert;
        } else if (idx == 2) {
            v2 = vert;
        } else {
            throw std::runtime_error("Invalid Vertex Index");
        }
    }
    /* AABB */
    Vec2f getXYMin() const {
        return Vec2f(std::min(v0.position.x(), std::min(v1.position.x(), v2.position.x())),
                     std::min(v0.position.y(), std::min(v1.position.y(), v2.position.y())));
    }
    Vec2f getXYMax() const {
        return Vec2f(std::max(v0.position.x(), std::max(v1.position.x(), v2.position.x())),
                     std::max(v0.position.y(), std::max(v1.position.y(), v2.position.y())));
    }
    /* Interpolation Functions */
    bool isInside(Vec3f pos) const;
    bool isInsidefor2D(Vec3f pos) const;
    Vec3f getInterpolationWeights(Vec3f pos) const;
    Vec3f getInterpolationWeightsfor2D(Vec3f pos) const;

    /* Material */
    void setMaterial(std::shared_ptr<Materials> mat) {
        material = mat;
    }
    std::shared_ptr<Materials> getMaterial() const {
        return material;
    }
};

#endif // GEOMETRY_HPP_