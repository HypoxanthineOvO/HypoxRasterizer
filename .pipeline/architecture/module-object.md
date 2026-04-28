# Module: Object and Scene

## Responsibility

`Modules/Object/` owns mesh primitives, material evaluation, OBJ loading,
local-to-world transforms, and the scene container.

## Files

- `geometry.hpp`, `geometry.cpp`: `Vertex` and `Triangle`, bounding boxes,
  inside-triangle checks, and barycentric interpolation.
- `object.hpp`, `object.cpp`: OBJ loading via tinyobjloader, local mesh storage,
  transformed world-space triangle generation, and material assignment.
- `materials.hpp`: abstract `Materials`, plus `ColorMaterial` and
  `TextureMaterial`.
- `scene.hpp`: simple vectors of shared object and light pointers.

## Data Model

- `Vertex` stores position, normal, and UV.
- `Triangle` owns three vertices and an optional material pointer.
- `Object` keeps original local-space triangles and transformed render triangles.
- `Scene` aggregates `Object` and `Light` instances for the rasterizer.

## Contracts

- OBJ faces must already be triangles; non-triangle faces raise an exception.
- OBJ normals are expected because `Object::loadObject` indexes normal data.
- `Object::localToWorld` must be called after loading to populate world-space
  triangles used by rendering.
- Materials are assigned per object and copied into triangles during vertex
  processing.

## Known Constraints

- The model transform is also applied directly to normals; inverse-transpose
  normal handling is not implemented.
- Scene getters return vectors by value, which is simple but copies shared
  pointer containers.
