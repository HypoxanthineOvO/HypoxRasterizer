#include "rasterizer.hpp"

void Rasterizer::Pass() {
    VertexProcessing();
    Rasterization();
    FragmentProcessing();
    DisplayToImage();
}

void Rasterizer::VertexProcessing() {
    /*
    Read the vertices from the scene and apply the camera transformation to them.
    */
    // 1. Generate the Matrix
    Mat4f view_matrix = camera->getViewMatrix(),
        projection_matrix = camera->getProjectionMatrix();
    // 2. Get All Vertices
    for (Object& obj : scene->getObjects()) {
        for (Triangle tri : obj.getTriangles()) {
            Triangle new_tri;
            for (int i = 0; i < 3; i++) {
                Vertex vert = tri.getVertex(i);
                Vertex new_vert = vert;
                // 3. Apply the Transformation
                Vec4f pos = projection_matrix * view_matrix * \
                    Vec4f(vert.position.x(), vert.position.y(), vert.position.z(), 1);

                new_vert.position = Vec3f(pos.x() / pos.w(), pos.y() / pos.w(), pos.z() / pos.w());
                Vec4f norm = view_matrix * \
                    Vec4f(vert.normal.x(), vert.normal.y(), vert.normal.z(), 0);
                new_vert.normal = Vec3f(norm.x(), norm.y(), norm.z());
                
                printf("Vertex %d\n", i);
                utils::printVec(new_vert.position);
                utils::printVec(new_vert.normal);
                new_tri.setVertex(i, new_vert);
            }
            triangle_buffer.push_back(new_tri);
        }
    }
}


void Rasterizer::Rasterization() {

}

void Rasterizer::FragmentProcessing() {

}

void Rasterizer::DisplayToImage() {
    // Write Color Buffer
    writeImageToFile(color_buffer, camera->getResolution(), "output.png");
}