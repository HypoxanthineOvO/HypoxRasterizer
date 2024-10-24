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
                Vertex vert(position_, normal_);
                tri.setVertex(v, vert);
            }
            addTriangleLocal(tri);
            index_offset += 3;
        }
    }
    
    puts("Object Loaded Successfully");
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
        }
        // Add Triangle to triangles
        addTriangle(tri);
    }
}