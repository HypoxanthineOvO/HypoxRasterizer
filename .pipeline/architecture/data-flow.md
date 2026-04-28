# Data Flow

## Initialization Flow

1. `main.cpp` accepts a config path from `argv[1]` or prompts on stdin.
2. `Rasterizer(config_path)` constructs `Config` and calls
   `initializeFromConfig`.
3. `Config::loadConfig` parses JSON into camera, material, object, and light
   structs.
4. `initializeFromConfig` creates:
   - `Camera` from camera config.
   - `Materials` map from material configs.
   - `Object` instances from OBJ paths, transformed into world space.
   - `Light` instances from light configs.
5. Each light receives the scene objects and generates its shadow map data.

## Render Flow

1. `Rasterizer::Pass` calls the render stages in fixed order.
2. `VertexProcessing` transforms each object triangle from world space to screen
   space while preserving original world-space triangles for shading.
3. `FragmentProcessing` rasterizes screen-space triangles into per-pixel buffers:
   depth, position, original position, normal, original normal, UV, and material.
4. `FragmentShading` evaluates material color and lighting for populated pixels.
5. `DisplayToImage` serializes color, depth, and normal buffers to PNG files.

## Shadow Flow

1. Point lights create six `ShadowMap` instances, one per axis direction.
2. Area lights create one `ShadowMap` aligned to the light normal.
3. `ShadowMap::generateDepthBuffer` rasterizes scene triangles from the light
   camera into a depth buffer.
4. `Light::isLighted` delegates world-space visibility checks to shadow maps.
5. Main fragment shading skips lights where `isLighted(position)` is false.

## Asset Flow

1. Scene JSON files under `configs/` reference OBJ and texture paths.
2. `Object::loadObject` reads OBJ assets through tinyobjloader.
3. `TextureMaterial::loadTexture` reads texture images through stb_image.
4. Render outputs and debug shadow-map outputs are written to the process
   working directory.
