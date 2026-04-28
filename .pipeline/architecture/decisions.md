# Architecture Decisions

## Current Decisions

- Use xmake as the build system and C++17 as the language standard.
- Build reusable renderer code into static library targets (`Utils` and
  `Rasterizer`) and keep `main.cpp` as a small CLI binary.
- Keep scene configuration in JSON files rather than hard-coding scenes.
- Use Eigen types globally through `Utils/cores.hpp`.
- Use tinyobjloader for OBJ mesh ingestion and stb for image IO.
- Represent point and area lights through a common `Light` interface.
- Use shadow maps for direct-light visibility checks.
- Keep the workflow architecture baseline in folder mode because the project has
  27 C++ source/header files and clear module boundaries.

## Open Technical Risks

- Automated tests are present as source files but not active build targets.
- Texture loading and lookup assume fixed 1024 by 1024 dimensions.
- Depth output normalization can divide by zero for uniform depth buffers.
- Normal transformation does not use inverse-transpose handling.
- Config parsing assumes required JSON fields exist and does not surface
  structured validation errors.
- Indirect lighting data structures exist, but the shading implementation is a
  TODO.

## Suggested First Planning Topics

- Restore a reliable build/test command for at least geometry, camera, and image
  IO behavior.
- Add config validation around scene loading before extending scene features.
- Factor duplicated rasterization logic between `Rasterizer` and `ShadowMap` if
  future changes touch both paths.
