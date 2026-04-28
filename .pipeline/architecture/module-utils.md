# Module: Utils and Config

## Responsibility

`Utils/` provides shared math types, constants, transforms, image IO, and JSON
configuration parsing used by the rest of the renderer.

## Files

- `Utils/cores.hpp`: Eigen aliases such as `Vec3f`, `Vec2i`, and `Mat4f`.
- `Utils/constant.hpp`: epsilon values, camera defaults, ambient light, VPL
  counts, and shadow-map defaults.
- `Utils/utils.hpp`: vector/matrix printing, vector equality, barycentric weight
  validation, and model-matrix generation.
- `Utils/Image/image.hpp`, `Utils/Image/image.cpp`: stb-backed image load/write
  helpers for RGBA input and PNG output.
- `Utils/Configs/configs.hpp`, `Utils/Configs/configs.cpp`: JSON schema structs
  and parser for camera, materials, objects, and lights.

## External Dependencies

- Eigen for all vector/matrix math.
- nlohmann_json for scene config parsing.
- stb_image and stb_image_write for texture loading and PNG output.

## Contracts

- Config JSON must include `Camera`, `Materials`, `Objects`, and `Lights`.
- Supported material types are `ColorMat` and `TextureMat`.
- Supported light types are `PointLight` and `AreaLight`.
- Image writing expects linear `Vec3f`/float buffers and applies gamma correction.

## Known Constraints

- `TextureMaterial` assumes a 1024 by 1024 texture layout after loading.
- `Config::loadConfig` does not currently validate missing fields before access.
- `utils::isValidWeight` throws on invalid values, so callers do not receive a
  normal `false` result for those cases.
