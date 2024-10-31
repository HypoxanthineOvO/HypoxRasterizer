#include "rasterizer.hpp"

void Rasterizer::Pass() {
    VertexProcessing();
    FragmentProcessing();
    FragmentShading();
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
        std::shared_ptr<Materials> mat = obj->getMaterial();
        for (Triangle tri : obj->getTriangles()) {
            Triangle new_tri;
            for (int i = 0; i < 3; i++) {
                Vertex vert = tri.getVertex(i);
                Vec4f org_pos = Vec4f(vert.position.x(), vert.position.y(), vert.position.z(), 1),
                    org_norm = Vec4f(vert.normal.x(), vert.normal.y(), vert.normal.z(), 0);
                
                
                // Apply the Transformation
                Vertex new_vert = vert;
                Vec4f pos = projection_matrix * view_matrix * org_pos;
                Vec4f norm = view_matrix * org_norm;
                
                new_vert.position = Vec3f(pos.x() / pos.w(), pos.y() / pos.w(), pos.z() / pos.w());
                new_vert.normal = Vec3f(norm.x(), norm.y(), norm.z());
                new_vert.uv = vert.uv;

                new_tri.setVertex(i, new_vert);
                new_tri.setMaterial(mat);
            }
            triangle_buffer.push_back(new_tri);
        }
    }
}



void Rasterizer::FragmentProcessing() {
    for (Triangle& tri: triangle_buffer) {
        // Get AABB Of the Triangle
        Vec2f min = tri.getXYMin(), max = tri.getXYMax();
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
        min_screen = (min_screen - Vec2i(1, 1)).cwiseMax(Vec2i(0, 0));
        max_screen = (max_screen + Vec2i(1, 1)).cwiseMin(Vec2i(camera->getWidth(), camera->getHeight()));

        // Rasterize the Triangle
        for (int x = min_screen.x(); x < max_screen.x(); x++) {
            for (int y = min_screen.y(); y < max_screen.y(); y++) {
                Vec3f pos = Vec3f(
                    2 * static_cast<float>(x) / width - 1,
                    2 * static_cast<float>(y) / height - 1,
                    0
                );
                
                if (tri.isInsidefor2D(pos)) {
                    // Interpolation Weights
                    Vec3f weights = tri.getInterpolationWeightsfor2D(pos);
                    // Check weights valid
                    if (!utils::isValidWeight(weights)) {
                        continue;
                    }
                    // Depth
                    float depth = weights.x() * tri.getVertex(0).position.z() +
                        weights.y() * tri.getVertex(1).position.z() +
                        weights.z() * tri.getVertex(2).position.z();
                    
                    // Check the Depth Buffer
                    if (std::abs(depth) >= depth_buffer[y * camera->getWidth() + x]) {
                        continue;
                    }
                    // Write to the Depth Buffer
                    depth_buffer[y * camera->getWidth() + x] = std::abs(depth);

                    // Position
                    Vec3f position = weights.x() * tri.getVertex(0).position +
                        weights.y() * tri.getVertex(1).position +
                        weights.z() * tri.getVertex(2).position;
                    position_buffer[y * camera->getWidth() + x] = position;
                    // Normal
                    Vec3f normal = weights.x() * tri.getVertex(0).normal +
                        weights.y() * tri.getVertex(1).normal +
                        weights.z() * tri.getVertex(2).normal;
                    normal_buffer[y * camera->getWidth() + x] = normal;
                    // uv
                    Vec2f uv = weights.x() * tri.getVertex(0).uv +
                        weights.y() * tri.getVertex(1).uv +
                        weights.z() * tri.getVertex(2).uv;
                    uv_buffer[y * camera->getWidth() + x] = uv;
                    // Material
                    material_buffer[y * camera->getWidth() + x] = tri.getMaterial();
                }
            }
        }
    }
}

void Rasterizer::FragmentShading() {
    for (int i = 0; i < camera->getWidth() * camera->getHeight(); i++) {
        // Get each fragment, and do the shading
        Vec3f position = position_buffer[i],
            normal = normal_buffer[i];
        Vec2f uv = uv_buffer[i];
        std::shared_ptr<Materials> mat = material_buffer[i];
        // Check if the material is nullptr
        if (mat == nullptr) {
            continue;
        }

        // Shading
        Vec3f vert_color = mat->evalColor(uv);
        float shininess = mat->evalShininess();
        // Ambient Light
        Vec3f color = AMBIENT.cwiseProduct(vert_color);
        // Diffuse and Specular Light
        for (std::shared_ptr<Light> light : scene->getLights()) {
            // Direct Shading
            std::vector<DirectVPL> d_vpls = light->getDirectVPLs();
            for (DirectVPL d_vpl : d_vpls) {
                Vec3f light_dir = d_vpl.position - position;
                light_dir.normalize();
                // Diffuse Shading
                float cos_theta_diffuse = light_dir.dot(normal.normalized());
                if (cos_theta_diffuse > 0) {
                    color += d_vpl.intensity.cwiseProduct(vert_color) * cos_theta_diffuse;
                }
                // Specular Shading
                Vec3f view_dir = camera->getPosition() - position;
                view_dir.normalize();
                Vec3f half_vec = (view_dir + light_dir).normalized();
                float cos_theta_specular = half_vec.dot(normal.normalized());
                if (cos_theta_specular > 0) {
                    color += d_vpl.intensity.cwiseProduct(vert_color) * pow(cos_theta_specular, shininess);
                }
            }
            // Indirect Shading
            std::vector<IndirectVPL> i_vpls = light->getIndirectVPLs();
            // TODO: Implement Indirect Shading

        }
        color_buffer[i] = color;
    }
}

void Rasterizer::DisplayToImage() {
    // Write Color Buffer
    writeImageToFile(color_buffer, camera->getResolution(), "color.png");
    // Write Depth Buffer
    writeImageToFile(depth_buffer, camera->getResolution(), "depth.png");
    // Write Normal Buffer
    writeImageToFile(normal_buffer, camera->getResolution(), "normal.png");
}