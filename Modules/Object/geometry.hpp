#ifndef GEOMETRY_HPP_
#define GEOMETRY_HPP_

#include "utils.hpp"

class Vertex {
public:
    /* Constructors */
    Vertex() = delete; // No Valid Default Constructor for Vertex
    Vertex(Vec3f pos) : position(pos), normal(1, 0, 0) {}
    Vertex(float x, float y, float z) : position(x, y, z), normal(1, 0, 0) {}
    Vertex(Vec3f pos, Vec3f norm) : position(pos), normal(norm) {}

    Vec3f position;
    Vec3f normal;
    Vec2f uv;
    Vec3f color;
    // TODO: Add Other Attributes
};

class Triangle {
public:
    /* Constructors */
    Triangle(): v0(Vec3f(0, 0, 0)), v1(Vec3f(0, 0, 0)), v2(Vec3f(0, 0, 0)) {}
    Triangle(Vertex v0, Vertex v1, Vertex v2) : v0(v0), v1(v1), v2(v2) {}

    /* Getters and Setters */
    Vertex getVertex(int idx) {
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
    /* Interpolation Functions */
    bool isInside(Vec3f pos);
    Vec3f getInterpolationWeights(Vec3f pos);
private:
    Vertex v0, v1, v2;
};

#endif // GEOMETRY_HPP_