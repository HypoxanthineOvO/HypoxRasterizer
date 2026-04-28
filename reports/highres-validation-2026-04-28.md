# 高清 Validation 渲染与 VPL/Shadow 对比报告

生成时间：2026-04-28 13:35（上海时间）

## 输出位置

- 主输出目录：`build/highres_validation_20260428/`
- Validation 总览图：`build/highres_validation_20260428/contact_sheets/validation_color_contact_sheet.png`
- Legacy 总览图：`build/highres_validation_20260428/contact_sheets/legacy_color_contact_sheet.png`
- VPL/Shadow 对比图：`build/highres_validation_20260428/contact_sheets/area_vpl_comparison_contact_sheet.png`
- VPL/Shadow 差异图：`build/highres_validation_20260428/contact_sheets/area_vpl_diff_vs_064_shadow_on.png`
- 渲染 manifest：`build/highres_validation_20260428/render_manifest.json`

## 总览图

![validation color contact sheet](../build/highres_validation_20260428/contact_sheets/validation_color_contact_sheet.png)

![legacy color contact sheet](../build/highres_validation_20260428/contact_sheets/legacy_color_contact_sheet.png)

## Validation 场景明细

| 场景 | 分辨率 | Color 输出 | 场景内容 | 验证重点 |
|---|---:|---|---|---|
| `fast_empty_scene` | 512x512 | `build/highres_validation_20260428/fast/fast_empty_scene/color.png` | 空场景，无几何、无光源、无材质。 | 验证空输入不会崩溃，color/G-Buffer 输出保持稳定。 |
| `fast_offscreen_triangle` | 512x512 | `build/highres_validation_20260428/fast/fast_offscreen_triangle/color.png` | `assets/Triangle/scene1.obj` 被平移到 `[100,100,0]`，无光源。 | 验证完全离屏几何不会产生错误片元。 |
| `fast_multi_light_shadow` | 512x512 | `build/highres_validation_20260428/fast/fast_multi_light_shadow/color.png` | 单三角形，蓝色 Phong 材质；一个 Point light 和一个 Directional light。 | 验证多光源、Point cube shadow 六面、Directional shadow、G-Buffer。 |
| `fast_point_smoke` | 512x512 | `build/highres_validation_20260428/fast/smoke_v2/color.png` | 单三角形，Point light。 | 基础 V2 schema、Point light、cube shadow、G-Buffer smoke。 |
| `standard_directional_smoke` | 512x512 | `build/highres_validation_20260428/standard/directional_smoke/color.png` | 单三角形，Directional light。 | Directional shadow map 和 shadow visibility。 |
| `standard_area_vpl_smoke` | 512x512 | `build/highres_validation_20260428/standard/area_vpl_smoke/color.png` | CornellBox：floor/ceiling/back/left/right/short_box/tall_box；顶部 area softbox；64 个 VPL。 | Area light、VPL 采样、Area shadow map、`light_0_direct_vpls.f32`。 |
| `standard_money_texture` | 512x512 | `build/highres_validation_20260428/standard/standard_money_texture/color.png` | `assets/Money/2.obj` + `assets/Money/2.png` 纹理；Point light。 | 纹理材质路径、UV、Point cube shadow。 |
| `standard_bunny_mesh` | 512x512 | `build/highres_validation_20260428/standard/standard_bunny_mesh/color.png` | `assets/Bunny/bunny.obj`，白色 Phong 材质；Point light。 | 中等规模 mesh、法线、cube shadow、G-Buffer。 |
| `standard_dragon_quick` | 256x256 | `build/highres_validation_20260428/standard/standard_dragon_quick/color.png` | 完整 Stanford Dragon mesh，低分辨率 quick 配置；Point light。 | Dragon 级 mesh 的可运行 standard 验证。 |
| `heavy_dragon_lowres_benchmark` | 256x256 | `build/highres_validation_20260428/heavy/heavy_dragon_lowres_benchmark/color.png` | 完整 Stanford Dragon mesh，heavy benchmark 配置；Point light。 | heavy/manual 层可执行入口，color/G-Buffer/cube shadow。 |

未渲染或跳过：

- `fast_missing_asset_error`：故意指向缺失资产，用于错误诊断，不属于高清输出。
- `heavy_orca_bistro_hook`：外部 ORCA Bistro hook，资产未下载时 clean skip。

## Legacy 测试场景

| 场景 | 分辨率 | Color 输出 | 场景内容 |
|---|---:|---|---|
| `TestScene1` | 512x512 | `build/highres_validation_20260428/legacy/TestScene1/color.png` | `assets/Triangle/scene1.obj`，绿色 ColorMat，Point light `[2,5,-1]`。 |
| `TestScene2` | 512x512 | `build/highres_validation_20260428/legacy/TestScene2/color.png` | `assets/Triangle/scene2.obj`，绿色 ColorMat，侧向相机和近距离 Point light。 |
| `TestScene3` | 512x512 | `build/highres_validation_20260428/legacy/TestScene3/color.png` | `assets/Triangle/scene3.obj`，紫色 ColorMat，Point light。 |

## VPL 与 Shadow Map 对比

![area vpl comparison](../build/highres_validation_20260428/contact_sheets/area_vpl_comparison_contact_sheet.png)

![area vpl diff](../build/highres_validation_20260428/contact_sheets/area_vpl_diff_vs_064_shadow_on.png)

对比场景统一使用 CornellBox/AreaVPL，512x512 输出：

| 变体 | Color 输出 | VPL 数量 | Shadow Map | 平均 RGB | 相对 `064_shadow_on` 平均绝对差 |
|---|---|---:|---|---|---:|
| `area_vpl_001_shadow_on` | `build/highres_validation_20260428/comparison/area_vpl_001_shadow_on/color.png` | 1 | 开 | `[124.3030, 125.8334, 114.5861]` | `0.3965` |
| `area_vpl_064_shadow_on` | `build/highres_validation_20260428/comparison/area_vpl_064_shadow_on/color.png` | 64 | 开 | `[124.4794, 125.9968, 114.6785]` | `0.0000` |
| `area_vpl_256_shadow_on` | `build/highres_validation_20260428/comparison/area_vpl_256_shadow_on/color.png` | 256 | 开 | `[124.4797, 125.9969, 114.6779]` | `0.0028` |
| `area_vpl_064_shadow_off` | `build/highres_validation_20260428/comparison/area_vpl_064_shadow_off/color.png` | 64 | 关 | `[192.0040, 192.2851, 178.6326]` | `65.9223` |

观察：

- 1 VPL 到 64 VPL 的差异主要集中在短盒边缘、背墙和局部高光位置；平均差不大，但最大差达到 49。
- 64 VPL 到 256 VPL 基本收敛，平均绝对差仅 `0.0028`，最大差为 1。
- 关闭 Shadow Map 后，场景整体明显变亮，平均 RGB 从约 `[124,126,115]` 上升到 `[192,192,179]`；阴影遮挡和接触暗部显著减少。
- 当前 Area light 的 shadow query 仍使用单张中心 shadow map；VPL 数量主要改变直接光采样分布，shadow 可见性不是逐 VPL 单独计算。

VPL 标注示例：

- `build/highres_validation_20260428/comparison/viz_area_vpl_064_shadow_on/light_0_direct_vpls_annotated.png`
- `build/highres_validation_20260428/comparison/viz_area_vpl_256_shadow_on/light_0_direct_vpls_annotated.png`

## 本轮代码小修正

为了支持真实 Shadow Map 开/关对比，本轮接通了已有 schema 字段：

- V2 `shadow.enabled` 现在会写入 legacy `ShadowEnabled`。
- legacy `Config` 会解析 `ShadowEnabled` / `shadowEnabled`。
- `Rasterizer` 仅在 `shadow_enabled=true` 时初始化并导出 shadow map；默认仍为开启，保持原有场景行为。
- 新增回归测试：`LightShadowTests` 中的 `Legacy config parses shadow enabled flag`。

## 3D 可视化规划草案

下一轮建议围绕 `tools/python` 继续追加，不另起 Python 环境。候选库：

- PyVista：官方定位是基于 VTK 的 Pythonic 3D plotting 和 mesh analysis，适合复杂网格、点云 glyph、离屏截图和交互窗口。
- Open3D：官方文档包含 mesh/point cloud、interactive visualization、non-blocking visualization、headless rendering、web visualizer 等路径，适合几何处理和相机/点云调试。
- trimesh：轻量纯 Python mesh loader/analysis，适合作为 OBJ/mesh 解析、包围盒、场景转换的辅助层，不一定作为主渲染 UI。

参考官方文档：

- PyVista: https://docs.pyvista.org/
- Open3D visualization: https://www.open3d.org/docs/latest/tutorial/visualization/index.html
- trimesh: https://trimesh.org/

建议里程碑：

1. `M13 3D Data Model`：把 V2 scene、OBJ mesh、camera、lights、VPL `.f32` 统一转为 Python scene graph。
2. `M14 Static 3D Preview`：生成 mesh + camera frustum + light/VPL glyph + shadow frustum 的离屏 PNG。
3. `M15 Interactive 3D Viewer`：支持旋转、选择对象、显示 G-Buffer 点反投影、VPL/Shadow map 联动。
4. `M16 Validation Integration`：把 3D 快照接入 validation runner/report，并为 heavy/external scene 做降采样显示策略。

首选执行路线：先用 `trimesh` 做资产读取/几何元数据，再用 `PyVista` 做主 3D 可视化；Open3D 作为 headless/点云/相机调试备选。
