#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "geometry.hpp"

class Object {
public:
    Object(): model_matrix(Mat4f::Identity()) {}
    Object(const std::string& file_name): model_matrix(Mat4f::Identity()) {
        loadObject(file_name);
    };

    /* Modify Functions */
    void addTriangle(Triangle tri) { triangles.push_back(tri); }
    void loadObject(const std::string& file_name);

    /* Transform Functions */
    void localToWorld(const Mat4f& model_mat);

    /* Getters */
    std::vector<Triangle> getTriangles() { return triangles; }
    Mat4f getModelMatrix() { return model_matrix; }

private:
    /* Raw Data in Local Space */
    std::vector<Triangle> triangles_local;
    
    /* Data for Rendering */
    Mat4f model_matrix;
    std::vector<Triangle> triangles;
};



#endif // OBJECT_HPP_