#include "rasterizer.hpp"

void Rasterizer::Pass() {
    VertexProcessing();
    FragmentProcessing();
    DisplayToImage();
}

void Rasterizer::VertexProcessing() {
    /*
    Read the vertices from the scene and apply the camera transformation to them.
    */
    // Generate the Matrix
    Mat4f view_matrix = camera->getViewMatrix(),
        projection_matrix = camera->getProjectionMatrix();
    // Get All Vertices
    for (std::shared_ptr<Object> obj : scene->getObjects()) {
        for (Triangle tri : obj->getTriangles()) {
            Triangle new_tri;
            for (int i = 0; i < 3; i++) {
                Vertex vert = tri.getVertex(i);
                Vec4f org_pos = Vec4f(vert.position.x(), vert.position.y(), vert.position.z(), 1),
                    org_norm = Vec4f(vert.normal.x(), vert.normal.y(), vert.normal.z(), 0);
                // Shading
                Vec3f color = AMBIENT; // Ambient Light
                for (std::shared_ptr<Light> light : scene->getLights()) {
                    // Direct Shading
                    std::vector<DirectVPL> d_vpls = light->getDirectVPLs();
                    for (DirectVPL d_vpl : d_vpls) {
                        Vec3f light_dir = d_vpl.position - vert.position;
                        light_dir.normalize();
                        // Diffuse Shading
                        float cos_theta = light_dir.dot(vert.normal);
                        if (cos_theta > 0) {
                            color += d_vpl.intensity * cos_theta;
                        }
                    }

                    std::vector<IndirectVPL> i_vpls = light->getIndirectVPLs();
                    // TODO: Indirect Shading
                }
                
                
                // Apply the Transformation
                Vertex new_vert = vert;
                Vec4f pos = projection_matrix * view_matrix * org_pos;
                new_vert.position = Vec3f(pos.x() / pos.w(), pos.y() / pos.w(), 0);
                Vec4f norm = view_matrix * org_norm;
                new_vert.normal = Vec3f(norm.x(), norm.y(), norm.z());
                new_vert.color = color;
                
                printf("Vertex %d\n", i);
                printf("\tPosition: ");
                utils::printVec(new_vert.position);
                printf("\tNormal: ");
                utils::printVec(new_vert.normal);
                printf("\tColor: ");
                utils::printVec(color);
                new_tri.setVertex(i, new_vert);
            }
            triangle_buffer.push_back(new_tri);
        }
    }
}



void Rasterizer::FragmentProcessing() {
    for (Triangle& tri: triangle_buffer) {
        // Get AABB Of the Triangle
        Vec2f min = tri.getXYMin(), max = tri.getXYMax();
        printf("AABB: ");
        utils::printVec(min);
        utils::printVec(max);
        float width = static_cast<float>(camera->getWidth()),
            height = static_cast<float>(camera->getHeight());
        Vec2i min_screen = Vec2i(
            static_cast<int>((min.x() + 1) / 2 * width),
            static_cast<int>((min.y() + 1) / 2 * height)
        ),
            max_screen = Vec2i(
                static_cast<int>((max.x() + 1) / 2 * width),
                static_cast<int>((max.y() + 1) / 2 * height)
            );
        // Clip the bounding box
        min_screen = min_screen.cwiseMax(Vec2i(0, 0));
        max_screen = max_screen.cwiseMin(Vec2i(camera->getWidth(), camera->getHeight()));
        printf("Screen Space AABB: ");
        printf("(%d, %d) -> (%d, %d)\n", min_screen.x(), min_screen.y(), max_screen.x(), max_screen.y());
    
        // Rasterize the Triangle
        for (int x = min_screen.x(); x < max_screen.x(); x++) {
            for (int y = min_screen.y(); y < max_screen.y(); y++) {
                Vec3f pos = Vec3f(
                    2 * static_cast<float>(x) / width - 1,
                    2 * static_cast<float>(y) / height - 1,
                    0
                );
                
                if (tri.isInside(pos)) {
                    Vec3f weights = tri.getInterpolationWeights(pos);
                    Vec3f color = weights.x() * tri.getVertex(0).color +
                        weights.y() * tri.getVertex(1).color +
                        weights.z() * tri.getVertex(2).color;
                    // Write to the Color Buffer
                    printf("Pixel (%d, %d)\n", x, y);
                    printf("\tColor: ");
                    utils::printVec(color);
                    color_buffer[y * camera->getWidth() + x] = color;
                }
            }
        }
    }
}

void Rasterizer::DisplayToImage() {
    // Write Color Buffer
    writeImageToFile(color_buffer, camera->getResolution(), "output.png");
}