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
    // TODO: Add Other Attributes
};

class Triangle {
public:
    /* Constructors */
    Triangle() = delete; // No Valid Default Constructor for Triangle
    Triangle(Vertex v0, Vertex v1, Vertex v2) : v0(v0), v1(v1), v2(v2) {}
    /* Interpolation Functions */
    bool isInside(Vec3f pos);
    Vec3f getInterpolationWeights(Vec3f pos);
private:
    Vertex v0, v1, v2;
};

#endif // GEOMETRY_HPP_