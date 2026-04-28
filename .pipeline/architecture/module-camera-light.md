# Module: Camera, Light, and Shadow Maps

## Responsibility

Camera and lighting modules provide view/projection transforms, direct-light
sampling, and depth-map based visibility tests.

## Camera

Files:

- `Modules/Camera/camera.hpp`
- `Modules/Camera/camera.cpp`

Key behavior:

- `Camera::lookAt` derives forward/right/up vectors from a position and target.
- `Camera::getViewMatrix` builds a right-handed view transform.
- `Camera::getProjectionMatrix` builds a perspective-to-orthographic projection
  matrix using focal length, FOV, aspect ratio, and default far plane.

## Lights

Files:

- `Modules/Light/light.hpp`
- `Modules/Light/light.cpp`

Key behavior:

- `Light` is the polymorphic base for direct and indirect VPL storage.
- `PointLight` creates one direct VPL and six directional shadow maps.
- `AreaLight` samples a grid of direct VPLs across the area light surface and
  uses one shadow map.

## Shadow Maps

Files:

- `Modules/Light/shadowmap.hpp`
- `Modules/Light/shadowmap.cpp`

Key behavior:

- `ShadowMap::initialize` creates a camera from light position and direction.
- `ShadowMap::generateDepthBuffer` rasterizes scene triangles into a depth map.
- `ShadowMap::isLighted` projects a world-space position into the shadow map and
  compares depth with `SHADOW_MAP_BIAS`.
- `ShadowMap::showShadowMap` writes normalized depth debug output.

## Known Constraints

- Indirect VPL storage exists but indirect shading is not implemented.
- Shadow-map generation duplicates rasterization logic from the main rasterizer.
- Depth normalization can divide by zero when all depth values are equal.
