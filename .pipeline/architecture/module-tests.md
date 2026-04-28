# Module: Tests and Debug Assets

## Responsibility

`Tests/` contains small standalone programs for manually checking camera,
image, object, and triangle behavior. `debug/` contains scene/debug artifacts
used during development.

## Test Files

- `Tests/CameraTest.cpp`: exercises camera construction, `lookAt`, view matrix,
  and projection matrix output.
- `Tests/ImageTest.cpp`: writes synthetic RGB and grayscale PNGs.
- `Tests/ObjectTest.cpp`: loads a sample OBJ asset.
- `Tests/TriangleTest.cpp`: checks triangle containment and interpolation.
- `Tests/TriangleTestByPython/`: C++ and Python triangle comparison harness.

## Build Status

The `xmake.lua` file contains target definitions for these test programs, but
they are commented out. The project currently has no active one-command test
target.

## Debug Assets

- `debug/*.json`: experimental or merged scene configs.
- `debug/merged.obj`: merged OBJ artifact.
- `debug/OBJ_Merger.py`: helper script for OBJ merging.

## Workflow Implication

For TDD milestones, the first implementation task should usually activate or
replace the relevant test target before relying on `tests_pass` as a gate.
