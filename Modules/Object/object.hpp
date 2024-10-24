#ifndef OBJECT_HPP_
#define OBJECT_HPP_

#include "geometry.hpp"
#include "materials.hpp"

class Object {
public:
    Object(): model_matrix(Mat4f::Identity()), material(nullptr) {};
    Object(const std::string& file_name): model_matrix(Mat4f::Identity()), material(nullptr) {
        loadObject(file_name);
    };
    Object(const std::string& file_name, std::shared_ptr<Materials> mat): model_matrix(Mat4f::Identity()), material(mat) {
        loadObject(file_name);
    };

    /* Modify Functions */
    void addTriangle(Triangle tri) { triangles.push_back(tri); }
    void loadObject(const std::string& file_name);
    void setMaterial(std::shared_ptr<Materials> mat) { material = mat; }

    /* Transform Functions */
    void localToWorld(const Mat4f& model_mat);

    /* Getters */
    std::vector<Triangle> getTriangles() { return triangles; }
    Mat4f getModelMatrix() { return model_matrix; }
    std::shared_ptr<Materials> getMaterial() { return material; }

private:
    /* Raw Data in Local Space */
    std::vector<Triangle> triangles_local;
    void addTriangleLocal(Triangle tri) { triangles_local.push_back(tri); }

    /* Data for Rendering */
    Mat4f model_matrix;
    std::vector<Triangle> triangles;

    /* Material */
    std::shared_ptr<Materials> material;
};



#endif // OBJECT_HPP_