# Architecture Baseline

## Project Overview

HypoxRasterizer is a C++17 software rasterizer. It loads a JSON scene
configuration, builds camera, material, object, and light state, rasterizes OBJ
triangle meshes into screen-space buffers, shades visible fragments with direct
lighting and shadow maps, and writes PNG debug/output images.

## Detected Stack

- Language: C++17
- Build system: xmake
- Dependencies: Eigen, stb, nlohmann_json, OpenMP, tinyobjloader
- Entrypoint: `main.cpp`
- Main binary target: `HypoxRasterizer`
- Active libraries: `Utils`, `Rasterizer`
- Tests: standalone C++ test sources under `Tests/`; xmake targets are present
  in comments and are not currently active.
- CI: none detected.

## Directory Structure

- `main.cpp`: CLI entrypoint that reads a config path and runs one raster pass.
- `xmake.lua`: build configuration and dependency declarations.
- `configs/`: JSON scene configurations.
- `assets/`: OBJ/MTL mesh assets consumed by scene configs.
- `Utils/`: Eigen aliases, constants, math helpers, image IO, and config parsing.
- `Modules/Camera/`: camera orientation and view/projection matrices.
- `Modules/Object/`: geometry primitives, OBJ loading, scene container, materials.
- `Modules/Light/`: light abstractions, direct VPL sampling, and shadow maps.
- `Rasterizer/`: main pipeline orchestration and framebuffer logic.
- `Tests/`: standalone exploratory test programs.
- `debug/`: development assets and generated/experimental files.

## Module Map

- [Utils and Config](module-utils.md)
- [Object and Scene](module-object.md)
- [Camera, Light, and Shadow Maps](module-camera-light.md)
- [Rasterizer Pipeline](module-rasterizer.md)
- [Tests and Debug Assets](module-tests.md)
- [Data Flow](data-flow.md)
- [Architecture Decisions](decisions.md)

## Key Interfaces

- `Config::loadConfig(path)`: parses scene JSON into camera, material, object,
  and light config structs.
- `Rasterizer::Rasterizer(config_path)`: builds runtime scene state from config.
- `Rasterizer::Pass()`: runs vertex processing, fragment processing, shading,
  and image output.
- `Object::loadObject(path)`: loads OBJ triangles through tinyobjloader.
- `Camera::getViewMatrix()` and `Camera::getProjectionMatrix()`: transform
  world coordinates into clip/NDC space.
- `Light::initShadowMap(...)`, `Light::isLighted(...)`: shadow-map-backed
  visibility for point and area lights.

## Operational Notes

- Default usage is `xmake` followed by `./HypoxRasterizer <config.json>`.
- The renderer writes `color.png`, `depth.png`, `normal.png`, and shadow-map PNGs
  into the working directory.
- Config paths in JSON files are relative to the process working directory.
- No automated regression command is currently wired into the build.
