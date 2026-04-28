# HypoxRasterizer

当前版本：v0.1.0

HypoxRasterizer 是一个 CPU 光栅器实验项目，当前重点是为渲染科研实验输出可复现的图像、G-Buffer、Depth Map、Shadow Map、VPL 调试数据和 IndirectVPL 间接光采样结果。

## 功能概览

- Legacy JSON 和 V2 JSON 场景配置。
- 命令行指定 config、输出目录、输出 buffer、渲染分辨率。
- 结构化输出目录：`outputs/<scene-name>/{color.png,gbuffer/,shadow/}`。
- Float G-Buffer：world/view position、world/view normal、linear depth、albedo、roughness、metallic、material id、shadow visibility、indirect VPL radiance。
- 点光源 Cube Shadow Map、方向光 Shadow Map、面积光 direct VPL 采样。
- 实验性 first-bounce IndirectVPL：从光源采样首个命中点并在相机可见点 gather 间接光。
- validation suite：fast / standard / heavy 三档场景。
- Python/uv 工具：输出统计、baseline 对比、2D heatmap/VPL overlay、3D 静态预览和报告。

## 环境要求

- Linux/macOS shell 环境。
- C++17 编译器。
- `xmake`。
- Python 3.10+。
- `uv`，用于运行 `tools/python` 下的可视化和统计工具。

首次构建：

```sh
xmake
```

如果你只想确认工程能跑：

```sh
xmake run BaselineTests
xmake run SceneCliOutputTests
xmake run GBufferTests
xmake run LightShadowTests
xmake run VPLAreaTests
xmake run IndirectVPLTests
```

Python 工具测试：

```sh
PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests
```

## 最快渲染

点光源 smoke：

```sh
xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root outputs --buffers color,gbuffer,shadow
```

方向光 smoke：

```sh
xmake run HypoxRasterizer --config configs/DirectionalSmoke.v2.json --output-root outputs --buffers color,gbuffer,shadow
```

面积光 direct VPL smoke：

```sh
xmake run HypoxRasterizer --config configs/AreaVPLSmoke.v2.json --output-root outputs --buffers color,gbuffer,shadow
```

Cornell first-bounce IndirectVPL：

```sh
xmake run HypoxRasterizer --config configs/validation/standard/cornell_indirect_vpl.v2.json --output-root outputs --buffers color,gbuffer,shadow
```

输出会进入：

```text
outputs/<scene-name>/
  color.png
  gbuffer/
  shadow/
```

注意：底层 legacy renderer 仍会在仓库根目录短暂生成 `color.png`、`depth.png`、`normal.png` 和 shadow PNG；结构化目录里的内容才是当前推荐使用的输出。根目录 PNG 已被 `.gitignore` 忽略，可以直接删除。

## 命令行参数

渲染入口：

```sh
xmake run HypoxRasterizer --config <config.json> [--output-root outputs] [--buffers color,gbuffer,shadow] [--resolution WIDTHxHEIGHT]
```

参数说明：

| 参数 | 说明 |
|---|---|
| `--config <path>` | 场景配置路径。支持 legacy JSON 和 V2 JSON。也可以把 config path 当作第一个位置参数。 |
| `--output-root <dir>` | 输出根目录，默认是 `outputs`。最终目录是 `<dir>/<scene-name>/`。 |
| `--buffers <list>` | 逗号分隔的输出组。常用值是 `color`、`gbuffer`、`shadow`。不传时等价于全部输出。 |
| `--resolution WIDTHxHEIGHT` | 覆盖配置文件里的相机分辨率，例如 `512x512`、`1280x720`。 |
| `--debug` / `--verbose` | 已解析的调试开关，当前主要保留给后续扩展。 |

分辨率示例：

```sh
xmake run HypoxRasterizer --config configs/AreaVPLSmoke.v2.json --output-root outputs --buffers color,gbuffer,shadow --resolution 512x512
xmake run HypoxRasterizer --config configs/validation/standard/dragon_quick.v2.json --output-root outputs --buffers color,gbuffer,shadow --resolution 1024x1024
xmake run HypoxRasterizer --config configs/validation/standard/money_texture.v2.json --output-root outputs --buffers color --resolution 800x600
```

只输出颜色图：

```sh
xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root outputs --buffers color --resolution 256x256
```

输出颜色和 G-Buffer，不输出 shadow：

```sh
xmake run HypoxRasterizer --config configs/SmokeTest.v2.json --output-root outputs --buffers color,gbuffer --resolution 256x256
```

## 输出文件

`color.png` 是最终颜色图。

`gbuffer/` 中的 `.f32` 是权威 float buffer，PNG 只是预览。`.f32` 格式为：

```text
HYPORASTER_F32_V1
<buffer-name>
<width> <height> <channels>
<float32 binary payload>
```

当前 G-Buffer 通道：

| 文件 | 通道数 | 内容 |
|---|---:|---|
| `world_position.f32` | 3 | 世界空间位置 |
| `view_position.f32` | 3 | View 空间位置 |
| `linear_depth.f32` | 1 | 线性深度 |
| `world_normal.f32` | 3 | 世界空间法线 |
| `view_normal.f32` | 3 | View 空间法线 |
| `albedo.f32` | 3 | 材质基础色 |
| `material_id.f32` | 1 | 材质 id |
| `roughness.f32` | 1 | 粗糙度 |
| `metallic.f32` | 1 | 金属度 |
| `shadow_visibility.f32` | 1 | 直接光 shadow visibility |
| `indirect_vpl_radiance.f32` | 3 | IndirectVPL gather 后的间接光贡献 |
| `linear_depth_preview.png` | 3 | 线性深度预览图 |
| `depth.png` / `normal.png` | 3 | legacy depth/normal preview |

`shadow/` 中常见文件：

| 文件 | 内容 |
|---|---|
| `PointlightShadowMap_<i>_face_<axis>.png` | 点光源 cube shadow 六个面 |
| `DirectionalLightShadowMap_<i>.png` | 方向光 shadow map |
| `ArealightShadowMap_<i>.png` | 面积光 shadow map |
| `light_<i>_direct_vpls.f32` | 面积光 direct VPL 样本，记录 position 和 intensity |
| `light_<i>_indirect_vpls.f32` | first-bounce IndirectVPL 样本，记录 position、normal、flux、radius、bounce、source light、flags |

## 当前可用场景

### 顶层示例场景

| 配置 | 类型 | 说明 |
|---|---|---|
| `configs/SmokeTest.v2.json` | V2 | 快速点光源 smoke，含 cube shadow 和 G-Buffer |
| `configs/DirectionalSmoke.v2.json` | V2 | 方向光 shadow smoke |
| `configs/AreaVPLSmoke.v2.json` | V2 | Cornell 风格面积光 direct VPL smoke |
| `configs/SmokeTest.json` | legacy | legacy 点光源 smoke |

### Legacy 经典场景

这些配置仍可直接渲染，也支持 `--resolution` 覆盖：

| 配置 | 资产/目的 |
|---|---|
| `configs/TestScene1.json`、`configs/TestScene2.json`、`configs/TestScene3.json` | 三角形和基础 corner case |
| `configs/TwoLight.json` | 多光源基础测试 |
| `configs/ShadowMap.json` | shadow map 测试 |
| `configs/CornellBox.json` | Cornell Box |
| `configs/Bunny.json` | Stanford Bunny |
| `configs/Dragon.json` | Stanford Dragon |
| `configs/Money.json` | 带纹理 Money mesh |
| `configs/Nylon.json`、`configs/Nylon2.json`、`configs/LX.json` | 其他内部模型 |

示例：

```sh
xmake run HypoxRasterizer --config configs/CornellBox.json --output-root outputs --buffers color,gbuffer,shadow --resolution 512x512
xmake run HypoxRasterizer --config configs/Dragon.json --output-root outputs --buffers color,gbuffer,shadow --resolution 512x512
```

### Validation fast 场景

fast 场景用于日常快速检查和 corner case：

| Scene id | 配置 | 说明 |
|---|---|---|
| `fast_empty_scene` | `configs/validation/fast/empty_scene.v2.json` | 空场景，验证空输出不崩溃 |
| `fast_offscreen_triangle` | `configs/validation/fast/offscreen_triangle.v2.json` | 视野外几何 |
| `fast_multi_light_shadow` | `configs/validation/fast/multi_light_shadow.v2.json` | 点光源 + 方向光 mixed shadow |
| `fast_missing_asset_error` | `configs/validation/fast/missing_asset_error.v2.json` | 预期失败的 missing asset 诊断，不默认运行 |
| `fast_point_smoke` | `configs/SmokeTest.v2.json` | 默认点光源 smoke |

Dry-run fast 默认场景：

```sh
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --tier fast
```

真实渲染 fast 默认场景：

```sh
xmake
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier fast --render --resolution 128x128 --output-root build/validation_outputs
```

### Validation standard 场景

standard 场景用于比较正式的功能验证：

| Scene id | 配置 | 重点 |
|---|---|---|
| `standard_directional_smoke` | `configs/DirectionalSmoke.v2.json` | 方向光 shadow |
| `standard_area_vpl_smoke` | `configs/AreaVPLSmoke.v2.json` | Cornell、面积光、direct VPL |
| `standard_cornell_indirect_vpl` | `configs/validation/standard/cornell_indirect_vpl.v2.json` | Cornell、点光源、cube shadow、IndirectVPL |
| `standard_money_texture` | `configs/validation/standard/money_texture.v2.json` | 纹理材质 |
| `standard_money_indirect_vpl` | `configs/validation/standard/money_indirect_vpl.v2.json` | 纹理资产 + IndirectVPL，不默认运行 |
| `standard_bunny_mesh` | `configs/validation/standard/bunny_mesh.v2.json` | Bunny 中等 mesh |
| `standard_dragon_quick` | `configs/validation/standard/dragon_quick.v2.json` | Dragon quick |

运行全部 standard：

```sh
xmake
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier standard --render --resolution 256x256 --output-root build/validation_outputs
```

只跑 VPL 相关：

```sh
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier standard --tag vpl --render --resolution 256x256
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier standard --tag indirect_vpl --render --resolution 128x128
```

### Validation heavy 场景

heavy 场景默认不运行，适合 release/benchmark：

| Scene id | 配置 | 说明 |
|---|---|---|
| `heavy_dragon_lowres_benchmark` | `configs/validation/heavy/dragon_lowres_benchmark.v2.json` | Dragon 低分辨率 benchmark |
| `heavy_dragon_indirect_vpl_quick` | `configs/validation/heavy/dragon_indirect_vpl_quick.v2.json` | Dragon + IndirectVPL quick |
| `heavy_orca_bistro_hook` | `configs/validation/heavy/orca_bistro_placeholder.v2.json` | 外部 ORCA Bistro hook，缺资源时 clean skip |

运行 heavy：

```sh
xmake
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier heavy --render --resolution 128x128 --output-root build/heavy_validation
```

## V2 场景配置怎么写

V2 顶层结构：

```json
{
  "scene": {"name": "my_scene"},
  "camera": {
    "resolution": [512, 512],
    "position": [0, 1, 3],
    "target": [0, 0.8, 0],
    "focalLength": 1,
    "fov": 45
  },
  "materials": [],
  "objects": [],
  "lights": [],
  "outputs": {"root": "outputs", "buffers": ["color", "gbuffer", "shadow"]}
}
```

注意：

- `scene.name` 决定输出目录名。
- `camera.resolution` 是默认分辨率，命令行 `--resolution WIDTHxHEIGHT` 会覆盖它。
- `materials[].id` 被 `objects[].material` 引用。
- `objects[].mesh` 可以使用相对路径，例如 `./assets/CornellBox/floor.obj`。
- `objects[]` 支持 `translation`、`rotation`、`scale`，省略时分别使用 `[0,0,0]`、`[0,0,0]`、`[1,1,1]`。
- `outputs` 字段是配置层面的记录；实际运行时以 CLI 的 `--output-root` 和 `--buffers` 为准。

材质示例：

```json
{"id": "white", "type": "phong", "baseColor": [0.8, 0.8, 0.78], "roughness": 0.65, "metallic": 0.0}
```

纹理材质示例：

```json
{"id": "money", "type": "texture", "texture": "./assets/Money/2.png", "roughness": 0.5, "metallic": 0.0}
```

点光源：

```json
{
  "name": "key",
  "type": "point",
  "position": [0, 1.8, 0],
  "intensity": [1.8, 1.8, 1.8],
  "shadow": {"enabled": true}
}
```

方向光：

```json
{
  "name": "sun",
  "type": "directional",
  "direction": [-0.4, -1.0, -0.2],
  "intensity": [1.5, 1.5, 1.5],
  "shadow": {"enabled": true}
}
```

面积光 direct VPL：

```json
{
  "name": "softbox",
  "type": "area",
  "position": [0, 1.85, 0],
  "normal": [0, -1, 0],
  "size": [0.9, 0.6],
  "intensity": [2.5, 2.5, 2.5],
  "vpl": {"samples": 64, "seed": 17, "stochastic": false},
  "shadow": {"enabled": true}
}
```

IndirectVPL：

```json
{
  "name": "indirect_key",
  "type": "point",
  "position": [0, 1.85, 0],
  "intensity": [1.8, 1.8, 1.8],
  "shadow": {"enabled": true},
  "indirectVPL": {
    "enabled": true,
    "samples": 64,
    "seed": 19,
    "bounces": 1,
    "visibility": false,
    "clamp": 0.6,
    "radius": 0.2,
    "sourceLights": [0]
  }
}
```

当前 IndirectVPL 是实验性的 one-bounce first-hit sampler，不是完整全局光照。`visibility` 已接入，但很多验证场景为了先观察能量分布默认关闭。

## Python 可视化和统计

Python 工具统一在 `tools/python` 下，用 uv 管理。

2D 可视化整个输出目录：

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-viz scene --root outputs/area_vpl_smoke --output-dir build/viz/area_vpl
```

生成 3D 静态预览：

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-viz-3d-static --config configs/AreaVPLSmoke.v2.json --scene-output-root outputs/area_vpl_smoke --output build/viz/area_vpl_3d.png
```

生成 2D/3D 联合报告：

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-viz-3d-report --config configs/validation/standard/cornell_indirect_vpl.v2.json --scene-output-root outputs/standard_cornell_indirect_vpl --output-dir build/viz/cornell_indirect --point-stride 4
```

收集输出统计：

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-collect-stats --root outputs/area_vpl_smoke --output build/stats/area_vpl.json
```

比较 baseline：

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-compare-baseline --baseline validation/baselines/standard_area_vpl_smoke.json --current-root outputs/area_vpl_smoke
```

外部场景 metadata/fetch hook：

```sh
PYTHONPATH=tools/python uv run --project tools/python hypo-fetch-scenes --manifest validation/external_sources.yaml --dry-run
```

7890、7891、7892 是当前批准使用的代理端口，外部资源策略见 `docs/external-scenes.md`。

## 推荐工作流

日常改代码：

```sh
xmake
xmake run BaselineTests
xmake run SceneCliOutputTests
xmake run GBufferTests
xmake run LightShadowTests
xmake run VPLAreaTests
xmake run IndirectVPLTests
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier fast --render --resolution 128x128
```

改材质、G-Buffer、shadow 或 light：

```sh
xmake
xmake run MaterialTests
xmake run GBufferTests
xmake run LightShadowTests
xmake run VPLAreaTests
xmake run IndirectVPLTests
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier standard --render --resolution 256x256
```

Release 前：

```sh
xmake
xmake run BaselineTests
xmake run SceneCliOutputTests
xmake run GBufferTests
xmake run MaterialTests
xmake run LightShadowTests
xmake run VPLAreaTests
xmake run ValidationSceneTests
xmake run ValidationSuiteTests
xmake run IndirectVPLTests
PYTHONPATH=tools/python uv run --project tools/python python -m unittest discover -s tools/python/tests
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --renderer ./HypoxRasterizer --tier standard --render --resolution 256x256
```

## 清理生成物

生成物默认不进入 git。常用清理命令：

```sh
rm -rf build outputs .xmake tools/python/.venv
rm -f color.png depth.png normal.png PointlightShadowMap_*.png DirectionalLightShadowMap_*.png ArealightShadowMap_*.png
find . -type d -name '__pycache__' -prune -exec rm -rf {} +
```

不要删除 `assets/Money/2.png` 这类资产纹理；它们是场景输入，不是输出。

## 更多文档

- `docs/scene-config-v2.md`：V2 scene schema。
- `docs/validation-scenes.md`：validation suite 规则和场景说明。
- `docs/output-baselines.md`：统计、hash 和容差 baseline。
- `docs/visualization.md`：2D 可视化。
- `docs/3d-visualization.md`：3D scene graph、静态预览和报告。
- `docs/external-scenes.md`：Bitterli/PBRT/ORCA 等外部资源 hook。
