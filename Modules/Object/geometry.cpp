#include "geometry.hpp"

bool Triangle::isInside(Vec3f pos) const {
    Vec3f v_0 = v0.position, v_1 = v1.position, v_2 = v2.position;
    Vec3f e_0 = v_1 - v_0, e_1 = v_2 - v_1, e_2 = v_0 - v_2;
    Vec3f c_0 = pos - v_0, c_1 = pos - v_1, c_2 = pos - v_2;
    Vec3f n = e_0.cross(e_1);
    if (
        (n.dot(e_0.cross(c_0)) >= 0 && n.dot(e_1.cross(c_1)) >= 0 && n.dot(e_2.cross(c_2)) >= 0) 
        ||
        (n.dot(e_0.cross(c_0)) <= 0 && n.dot(e_1.cross(c_1)) <= 0 && n.dot(e_2.cross(c_2)) <= 0)
    ){
        return true;
    }
    return false;
}
bool Triangle::isInsidefor2D(Vec3f pos) const {
    Vec3f v_0 = v0.position, v_1 = v1.position, v_2 = v2.position;
    v_0.z() = 0;
    v_1.z() = 0;
    v_2.z() = 0;
    Vec3f e_0 = v_1 - v_0, e_1 = v_2 - v_1, e_2 = v_0 - v_2;
    Vec3f c_0 = pos - v_0, c_1 = pos - v_1, c_2 = pos - v_2;
    Vec3f n = e_0.cross(e_1);
    if (
        (n.dot(e_0.cross(c_0)) >= 0 && n.dot(e_1.cross(c_1)) >= 0 && n.dot(e_2.cross(c_2)) >= 0) 
        ||
        (n.dot(e_0.cross(c_0)) <= 0 && n.dot(e_1.cross(c_1)) <= 0 && n.dot(e_2.cross(c_2)) <= 0)
    ){
        return true;
    }
    return false;
}
Vec3f Triangle::getInterpolationWeights(Vec3f pos) const {
    if (!isInside(pos)) {
        return Vec3f(-1, -1, -1);
    }
    Vec3f v_0 = v0.position, v_1 = v1.position, v_2 = v2.position;
    Vec3f e_0 = v_1 - v_0, e_1 = v_2 - v_1, e_2 = v_0 - v_2;
    Vec3f c_0 = pos - v_0, c_1 = pos - v_1, c_2 = pos - v_2;
    Vec3f n = e_0.cross(e_1);
    float w_0 = n.dot(e_1.cross(c_1)) / n.dot(n);
    float w_1 = n.dot(e_2.cross(c_2)) / n.dot(n);
    float w_2 = n.dot(e_0.cross(c_0)) / n.dot(n);

    float sum = w_0 + w_1 + w_2;
    w_0 /= sum;
    w_1 /= sum;
    w_2 /= sum;
    return Vec3f(w_0, w_1, w_2);
}
Vec3f Triangle::getInterpolationWeightsfor2D(Vec3f pos) const {
    if (!isInsidefor2D(pos)) {
        return Vec3f(-1, -1, -1);
    }
    Vec3f v_0 = v0.position, v_1 = v1.position, v_2 = v2.position;
    v_0.z() = 0;
    v_1.z() = 0;
    v_2.z() = 0;
    Vec3f e_0 = v_1 - v_0, e_1 = v_2 - v_1, e_2 = v_0 - v_2;
    Vec3f c_0 = pos - v_0, c_1 = pos - v_1, c_2 = pos - v_2;
    Vec3f n = e_0.cross(e_1);
    float w_0 = n.dot(e_1.cross(c_1)) / n.dot(n);
    float w_1 = n.dot(e_2.cross(c_2)) / n.dot(n);
    float w_2 = n.dot(e_0.cross(c_0)) / n.dot(n);

    // Normalize
    float sum = w_0 + w_1 + w_2;
    w_0 /= sum;
    w_1 /= sum;
    w_2 /= sum;
    return Vec3f(w_0, w_1, w_2);
}
