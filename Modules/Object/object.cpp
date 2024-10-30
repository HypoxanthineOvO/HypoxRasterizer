#include "object.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void Object::loadObject(const std::string& file_name) {
    printf("Loading Object: %s\n", file_name.c_str());

    /*
    Use tinyobjloader to load the object file
        Reference: https://blog.csdn.net/qjh5606/article/details/89075014
    */
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file_name.c_str())) {
        throw std::runtime_error(warn + err);
    }

    printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
    printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
    printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
    printf("# of shapes    = %d\n", (int)shapes.size());
    printf("# of materials = %d\n", (int)materials.size());

    // Load all vertex data
    for (size_t i = 0; i < shapes.size(); i++) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
            int fv = shapes[i].mesh.num_face_vertices[f];

            // Only Triangles are supported now
            if (fv != 3) {
                throw std::runtime_error("Only Triangles are supported");
            }

            Triangle tri;
            for (size_t v = 0; v < 3; v++) {
                tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
                Vec3f position_ (
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                );
                Vec3f normal_(
                    attrib.normals[3 * idx.normal_index + 0],
                    attrib.normals[3 * idx.normal_index + 1],
                    attrib.normals[3 * idx.normal_index + 2]
                );
                //printf("Texcoord Index: %d\n", idx.texcoord_index);
                // If no texcoord is provided, set it to (0, 0)
                Vec2f texcoord_(0, 0);
                if (attrib.texcoords.size()) {
                    texcoord_ = Vec2f(
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                    );
                }
                // Update the min_bound and max_bound
                if (triangles_local.size() == 0) {
                    min_bound_local = position_;
                    max_bound_local = position_;
                } else {
                    min_bound_local = min_bound_local.cwiseMin(position_);
                    max_bound_local = max_bound_local.cwiseMax(position_);
                }

                Vertex vert(position_, normal_, texcoord_);
                tri.setVertex(v, vert);
            }
            addTriangleLocal(tri);
            index_offset += 3;
        }
    }
    
    // Calculate the center
    center_local = (min_bound_local + max_bound_local) / 2;

    puts("Object Loaded Successfully");
    // Print the min_bound and max_bound of the object
    printf("Min Bound: ");
    utils::printVec(min_bound_local);
    printf("Max Bound: ");
    utils::printVec(max_bound_local);
    printf("Center: ");
    utils::printVec(center_local);
}

void Object::localToWorld(const Mat4f& model_mat) {
    // Clear Old triangles
    triangles.clear();
    // Apply Transformation
    model_matrix = model_mat;
    for (Triangle tri : triangles_local) {
        // Apply Transformation to Each Vertex's Position
        for (int i = 0; i < 3; i++) {
            Vertex vert = tri.getVertex(i);
            Vec4f pos(vert.position.x(), vert.position.y(), vert.position.z(), 1);
            Vec4f new_pos = model_mat * pos;
            vert.position = Vec3f(new_pos.x(), new_pos.y(), new_pos.z());
            tri.setVertex(i, vert);
        }
        // Apply Transformation to Each Vertex's Normal
        for (int i = 0; i < 3; i++) {
            Vertex vert = tri.getVertex(i);
            Vec4f norm(vert.normal.x(), vert.normal.y(), vert.normal.z(), 0);
            Vec4f new_norm = model_mat * norm;
            vert.normal = Vec3f(new_norm.x(), new_norm.y(), new_norm.z());
            tri.setVertex(i, vert);

            // Update the min_bound and max_bound
            if (triangles.size() == 0) {
                min_bound = vert.position;
                max_bound = vert.position;
            } else {
                min_bound = min_bound.cwiseMin(vert.position);
                max_bound = max_bound.cwiseMax(vert.position);
            }
        }
        // Add Triangle to triangles
        addTriangle(tri);
    }

    // Calculate the center
    center = (min_bound + max_bound) / 2;
    // Print the min_bound and max_bound of the object
    puts("Apply Transformation Successfully");
    printf("Min Bound: ");
    utils::printVec(min_bound);
    printf("Max Bound: ");
    utils::printVec(max_bound);
    printf("Center: ");
    utils::printVec(center);
}