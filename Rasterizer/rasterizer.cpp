#include "rasterizer.hpp"
#include "shadowmap.hpp"
#include <map>

/**
 * @brief 构造函数，从配置文件初始化光栅化器。
 * @param config_path 配置文件路径。
 * @note 初始化相机、场景、材质、光源以及各种缓冲区。
 */
Rasterizer::Rasterizer(const std::string& config_path) {
    Config config(config_path);
    initializeFromConfig(config);
    // Initialize Buffers.
    triangle_buffer.clear();

    // Initialize the Screen Space Buffer with -
    color_buffer.resize(camera->getWidth() * camera->getHeight(), Vec3f::Zero());
    depth_buffer.resize(camera->getWidth() * camera->getHeight(), 1);
    position_buffer.resize(camera->getWidth() * camera->getHeight(), Vec3f::Zero());
    normal_buffer.resize(camera->getWidth() * camera->getHeight(), Vec3f::Zero());
    org_position_buffer.resize(camera->getWidth() * camera->getHeight(), Vec3f::Zero());
    org_normal_buffer.resize(camera->getWidth() * camera->getHeight(), Vec3f::Zero());
    uv_buffer.resize(camera->getWidth() * camera->getHeight(), -Vec2f::Ones());
    material_buffer.resize(camera->getWidth() * camera->getHeight(), nullptr);
}

/**
 * @brief 根据配置文件初始化光栅化器。
 * @param config 配置对象。
 * @note 初始化相机、场景中的对象和光源，并为光源生成阴影贴图。
 */
void Rasterizer::initializeFromConfig(const Config& config) {
    // 1. Initialize Camera
    std::shared_ptr<Camera> cam = std::make_shared<Camera>(
        config.camera_config.position, config.camera_config.target
    );
    cam->setResolution(config.camera_config.resolution);
    cam->setFOV(config.camera_config.fov);

    // 2. Initialize Scene
    std::shared_ptr<Scene> scn = std::make_shared<Scene>();
    // 2.1. Initialize Materials
    std::map<std::string, std::shared_ptr<Materials>> materials;
    for (MaterialConfig mat_config : config.materials_config) {
        std::shared_ptr<Materials> mat;
        if (mat_config.type == Color_Mat) {
            // Color Material: Have base_color
            mat = std::make_shared<ColorMaterial>(
                mat_config.base_color, mat_config.shininess
            );
        } else if (mat_config.type == Texture_Mat) {
            mat = std::make_shared<TextureMaterial>(
                mat_config.texture_file_path, mat_config.shininess
            );
        }
        materials[mat_config.name] = mat;
    }
    // Material for Light
    materials["light"] = std::make_shared<ColorMaterial>(
        AMBIENT.cwiseInverse()
    );
    // 2.2. Initialize Objects
    for (ObjectConfig obj_config : config.objects_config) {
        std::shared_ptr<Object> obj = std::make_shared<Object>(
            obj_config.file_path
        );
        Mat4f trans_matrix = utils::generateModelMatrix(
            obj_config.translation, obj_config.rotation, obj_config.scale
        );
        obj->localToWorld(trans_matrix);
        obj->setMaterial(materials[obj_config.material]);
        scn->addObject(obj);
    }
    // 2.3. Initialize Lights
    for (LightConfig light_config : config.lights_config) {
        std::shared_ptr<Light> light;
        if (light_config.type == Point_Light) {
            light = std::make_shared<PointLight>(
                light_config.position, light_config.intensity
            );

            // Initialize Shadow Map
            std::vector<std::shared_ptr<Object>> objects = scn->getObjects();
            light->initShadowMap(DEFAULT_SHADOW_MAP_RESOLUTION, objects);
            //light->showShadowMap("PointlightShadowMap.png");
        }
        else if (light_config.type == Area_Light) {
            light = std::make_shared<AreaLight>(
                light_config.position, light_config.intensity,
                light_config.normal, light_config.size
            );
            // Initialize Shadow Map
            std::vector<std::shared_ptr<Object>> objects = scn->getObjects();
            light->initShadowMap(DEFAULT_SHADOW_MAP_RESOLUTION, objects);
            //light->showShadowMap("ArealightShadowMap.png");
            // Add an object for the area light
            // std::shared_ptr<Object> obj = std::make_shared<Object>(
            //     "assets/Objects/ground.obj"
            // );
            // obj->localToWorld(utils::generateModelMatrix(
            //     light_config.position + Vec3f(0, 0.01, 0), Vec3f(0, 180, 0), Vec3f(light_config.size.x(), 1, light_config.size.y())
            // ));
            // obj->setMaterial(materials["light"]);
            // scn->addObject(obj);
        }
        else {
            puts("Unknown Light Type");
            exit(1);
        }
        scn->addLight(light);
    }

    // 3. Initialize Rasterizer
    camera = cam;
    scene = scn;

    printf("Initialized Rasterizer with %ld objects and %ld lights\n", scene->getObjects().size(), scene->getLights().size());
}

/**
 * @brief 执行光栅化的主要流程。
 * @note 包括顶点处理、片元处理、片元着色和图像输出。
 */
void Rasterizer::Pass() {
    puts("Passing the Rasterizer");
    VertexProcessing();
    puts("Vertex Processing Done");
    FragmentProcessing();
    puts("Fragment Processing Done");
    FragmentShading();
    puts("Fragment Shading Done");
    DisplayToImage();
}

/**
 * @brief 顶点处理阶段。
 * @note 将场景中的顶点从世界坐标转换到屏幕空间。
 */
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
            org_triangle_buffer.push_back(tri);
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
            }
            new_tri.setMaterial(mat);
            triangle_buffer.push_back(new_tri);
        }
    }
}

/**
 * @brief 片元处理阶段。
 * @note 对每个三角形进行光栅化，计算每个像素的深度、法线、材质等信息。
 */
void Rasterizer::FragmentProcessing() {
    uint32_t triangle_cnt = triangle_buffer.size();
    for (int tid = 0; tid < triangle_cnt; tid++) {
        //for (Triangle &tri : triangle_buffer)
        Triangle& tri = triangle_buffer[tid];
        Triangle& org_tri = org_triangle_buffer[tid];
        // Get AABB Of the Triangle
        Vec2f min = tri.getXYMin(), max = tri.getXYMax();
        float width = static_cast<float>(camera->getWidth()),
                height = static_cast<float>(camera->getHeight());
        Vec2i min_screen = Vec2i(
                    static_cast<int>((min.x() + 1) / 2 * width),
                    static_cast<int>((min.y() + 1) / 2 * height)),
                max_screen = Vec2i(
                    static_cast<int>((max.x() + 1) / 2 * width),
                    static_cast<int>((max.y() + 1) / 2 * height));
        // Clip the bounding box
        min_screen = (min_screen - Vec2i(1, 1)).cwiseMax(Vec2i(0, 0));
        max_screen = (max_screen + Vec2i(1, 1)).cwiseMin(Vec2i(camera->getWidth(), camera->getHeight()));

        // Rasterize the Triangle
        for (int x = min_screen.x(); x < max_screen.x(); x++) {
            for (int y = min_screen.y(); y < max_screen.y(); y++) {
                Vec3f pos = Vec3f(
                    2 * static_cast<float>(x) / width - 1,
                    2 * static_cast<float>(y) / height - 1,
                    0);

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
                    if (std::abs((depth - 1) / 2) >= depth_buffer[y * camera->getWidth() + x]) {
                        continue;
                    }
                    // Write to the Depth Buffer
                    depth_buffer[y * camera->getWidth() + x] = std::abs((depth - 1) / 2);

                    /*
                    We save the original information (In the global / world space) of the fragment
                    We also save the information in the camera space
                    */
                    // Position
                    Vec3f position = weights.x() * tri.getVertex(0).position +
                                        weights.y() * tri.getVertex(1).position +
                                        weights.z() * tri.getVertex(2).position;
                    position_buffer[y * camera->getWidth() + x] = position;
                    Vec3f org_position = weights.x() * org_tri.getVertex(0).position +
                                        weights.y() * org_tri.getVertex(1).position +
                                        weights.z() * org_tri.getVertex(2).position;
                    org_position_buffer[y * camera->getWidth() + x] = org_position;
                    // Normal
                    Vec3f normal = weights.x() * tri.getVertex(0).normal +
                                    weights.y() * tri.getVertex(1).normal +
                                    weights.z() * tri.getVertex(2).normal;
                    normal_buffer[y * camera->getWidth() + x] = normal;
                    Vec3f org_normal = weights.x() * org_tri.getVertex(0).normal +
                                    weights.y() * org_tri.getVertex(1).normal +
                                    weights.z() * org_tri.getVertex(2).normal;
                    org_normal_buffer[y * camera->getWidth() + x] = org_normal;
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

/**
 * @brief 片元着色阶段。
 * @note 根据光照模型计算每个像素的颜色。
 */
void Rasterizer::FragmentShading() {
    for (int i = 0; i < camera->getWidth() * camera->getHeight(); i++) {
        // Get each fragment, and do the shading
        Vec3f position = org_position_buffer[i],
            normal = org_normal_buffer[i];
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
            if (!light->isLighted(position)) {
                continue;
            }

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

/**
 * @brief 将缓冲区中的数据保存为图像文件。
 * @note 包括颜色缓冲区、深度缓冲区和法线缓冲区。
 */
void Rasterizer::DisplayToImage() {
    std::vector<float> depth_buffer_normal = depth_buffer;
    // Write Color Buffer
    writeImageToFile(color_buffer, camera->getResolution(), "color.png");
    // Write Depth Buffer
    // Get min_value and max_value
    float min_value = 1, max_value = 0;
    for (int i = 0; i < camera->getWidth() * camera->getHeight(); i++) {
        min_value = std::min(min_value, depth_buffer[i]);
        max_value = std::max(max_value, depth_buffer[i]);
    }

    // Normalize the Depth Buffer
    for (int i = 0; i < camera->getWidth() * camera->getHeight(); i++) {
        depth_buffer_normal[i] = (depth_buffer[i] - min_value) / (max_value-min_value);
    }
    writeImageToFile(depth_buffer_normal, camera->getResolution(), "depth.png");
    // Write Normal Buffer
    writeImageToFile(normal_buffer, camera->getResolution(), "normal.png");
}