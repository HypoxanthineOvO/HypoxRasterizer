# Module: Rasterizer Pipeline

## Responsibility

`Rasterizer/` is the central render pipeline. It initializes scene state from
config, owns screen-space buffers, performs triangle rasterization, shades
fragments, and writes output images.

## Files

- `Rasterizer/rasterizer.hpp`
- `Rasterizer/rasterizer.cpp`

## Runtime State

- Camera pointer and scene pointer.
- Triangle buffers for transformed and original world-space geometry.
- Per-pixel buffers for color, depth, screen-space position, world-space
  position, screen-space normal, world-space normal, UV, and material pointer.

## Pipeline Stages

1. `initializeFromConfig`: creates camera, materials, objects, and lights from
   parsed JSON config. Light initialization also builds shadow maps.
2. `VertexProcessing`: transforms object triangles through camera view and
   projection matrices into NDC-like screen space.
3. `FragmentProcessing`: computes triangle AABBs, iterates covered pixels,
   tests 2D containment, interpolates attributes, and writes the closest
   fragment into buffers.
4. `FragmentShading`: evaluates material color, ambient light, direct light,
   specular terms, and shadow visibility.
5. `DisplayToImage`: writes color, depth, and normal images.

## Outputs

- `color.png`
- `depth.png`
- `normal.png`
- `PointlightShadowMap.png_*` for point light faces
- `ArealightShadowMap.png` for area lights

## Known Constraints

- No clipping stage exists beyond screen-space AABB clipping.
- The main pass is CPU scalar loops; OpenMP is declared as a dependency but is
  not used in the current source.
- `FragmentShading` has a TODO for indirect shading.
