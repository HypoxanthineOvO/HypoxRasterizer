# 高清 Validation 渲染汇报

生成时间：2026-04-28 13:28（上海时间）

## 概览

本轮使用统一 `512x512` 分辨率渲染 validation suite：

```sh
python3 tools/validation/validation_runner.py --manifest validation/scenes.yaml --all --include-heavy --render --resolution 512x512 --output-root build/highres_validation_outputs
```

结果：`12` 个场景被选中，`10` 个渲染通过，`2` 个按预期跳过。

- 通过：所有内部 fast / standard / heavy 可运行场景。
- 跳过：`fast_missing_asset_error`、`heavy_orca_bistro_hook`。
- 输出根目录：`build/highres_validation_outputs/`
- 总览图：`build/highres_visualizations/all_highres_color_contact_sheet.png`

## 高清 Color 图清单

| 场景 | Color 图 | 场景内容 | 光照/Shadow | 主要验证点 |
|---|---|---|---|---|
| `fast_empty_scene` | `build/highres_validation_outputs/fast/fast_empty_scene/color.png` | 空场景；无 objects、无 materials、无 lights；相机看向原点 | 无 | 空输入时仍能输出 color 和 G-Buffer |
| `fast_offscreen_triangle` | `build/highres_validation_outputs/fast/fast_offscreen_triangle/color.png` | 单三角形 asset `assets/Triangle/scene1.obj`，整体平移到 `[100,100,0]` 使其离屏 | 无 | 离屏几何不会导致错误，输出应基本为空 |
| `fast_multi_light_shadow` | `build/highres_validation_outputs/fast/fast_multi_light_shadow/color.png` | 单三角形，Phong 材质，蓝色系 base color | Point light + Directional light；输出 point cube faces 和 directional shadow map | 多光源、Point cube shadow、Directional shadow 同时存在 |
| `fast_point_smoke` | `build/highres_validation_outputs/fast/smoke_v2/color.png` | 基础三角形 smoke 场景 | Point light；输出 6 张 cube shadow face | 默认 fast smoke、CLI/V2 schema/output layout |
| `standard_directional_smoke` | `build/highres_validation_outputs/standard/directional_smoke/color.png` | 基础三角形 directional 场景 | Directional light；输出 `DirectionalLightShadowMap_0.png` | Directional light 和 directional shadow 输出路径 |
| `standard_area_vpl_smoke` | `build/highres_validation_outputs/standard/area_vpl_smoke/color.png` | CornellBox 风格房间：floor、ceiling、back、left、right、short box、tall box；白/红/绿 Phong 材质 | Area light softbox，位置 `[0,1.85,0]`，size `[0.9,0.6]`，VPL samples `64`，shadow enabled | Cornell/VPL、Area shadow map、`light_0_direct_vpls.f32`、G-Buffer |
| `standard_money_texture` | `build/highres_validation_outputs/standard/standard_money_texture/color.png` | Money mesh `assets/Money/2.obj`，纹理 `assets/Money/2.png` | Point light；输出 point cube shadow faces | 纹理材质路径、UV/texture sampling、Point shadow |
| `standard_bunny_mesh` | `build/highres_validation_outputs/standard/standard_bunny_mesh/color.png` | Bunny mesh `assets/Bunny/bunny.obj`，约 5k vertices，白色 Phong 材质 | Point light；输出 point cube shadow faces | 中等 mesh、G-Buffer、cube shadow |
| `standard_dragon_quick` | `build/highres_validation_outputs/standard/standard_dragon_quick/color.png` | Stanford Dragon `assets/Dragon/stanford_dragon.obj`，约 437k vertices，低预算 standard 配置 | Point light；输出 point cube shadow faces | Dragon 级 mesh 能较快作为 standard validation 运行 |
| `heavy_dragon_lowres_benchmark` | `build/highres_validation_outputs/heavy/heavy_dragon_lowres_benchmark/color.png` | 同 Stanford Dragon，heavy/benchmark 入口；本轮用 `512x512` 覆盖输出 | Point light；输出 point cube shadow faces | heavy benchmark 入口、性能预算、手动验证层 |

## 跳过项

- `fast_missing_asset_error`：故意指向 `assets/Missing/not_found.obj`，用于验证缺失资源诊断。
- `heavy_orca_bistro_hook`：外部 ORCA Bistro hook；需要下载 `assets/external/orca/bistro` 后再启用真实渲染。

## Area VPL / Shadow 对比

对比场景基于 `configs/AreaVPLSmoke.v2.json` 的 CornellBox 风格场景，额外生成了三组 `512x512` 对照：

| 对照 | Color 图 | 关键输出 |
|---|---|---|
| VPL 64 samples + shadow output | `build/highres_comparison_outputs/area_vpl_shadow_64samples/color.png` | `shadow/ArealightShadowMap_0.png`，`shadow/light_0_direct_vpls.f32`，shape `[64,1,6]` |
| VPL 4 samples + shadow output | `build/highres_comparison_outputs/area_vpl_shadow_4samples/color.png` | `shadow/ArealightShadowMap_0.png`，`shadow/light_0_direct_vpls.f32`，shape `[4,1,6]` |
| VPL 64 samples + no shadow output | `build/highres_comparison_outputs/area_vpl_no_shadow_output/color.png` | 无 `shadow/` 输出；仍输出 color 和 G-Buffer |

可视化图：

- 对比总览：`build/highres_visualizations/area_vpl_comparison_contact_sheet.png`
- 64-sample VPL overlay：`build/highres_visualizations/compare_area_vpl_64/light_0_direct_vpls_annotated.png`
- 4-sample VPL overlay：`build/highres_visualizations/compare_area_vpl_4/light_0_direct_vpls_annotated.png`
- 64-sample shadow sheet：`build/highres_visualizations/compare_area_vpl_64/shadow_contact_sheet.png`
- Diff：`build/highres_visualizations/64_samples_vs_4_samples_diff.png`
- Diff：`build/highres_visualizations/64_samples_vs_no_shadow_output_diff.png`

量化观察：

- `64 samples` vs `4 samples`：color 平均 RGB 绝对差约 `0.0521`，最大差 `6`。视觉上差异较弱，但 VPL overlay 明确显示采样点数量不同。
- `64 samples + shadow output` vs `no shadow output`：color 差异为 `0`。当前 `shadow.enabled=false`/不请求 shadow 输出主要影响 debug map 导出，而不是 color shading 开关。
- `shadow_visibility.f32` 在上述三组 Area VPL 对比中统计一致：mean `0.325012`，min `0`，max `1`。这说明当前 shadow map/visibility 更接近调试数据路径，尚未形成对最终 color 的强视觉开关。

结论：VPL 采样与 shadow debug buffer 已经可以被稳定导出和可视化；但如果后续希望“开/关 Shadow Map”在 color 图上产生明显视觉差异，需要把 shadow visibility 更严格接入 shading/BRDF 计算路径。

## 3D Python 可视化下一轮规划入口

下一轮建议进入正式 plan，目标是从当前 2D PNG 工具扩展为 3D/交互式实验视图：

1. 读取 `.f32` world_position、world_normal、shadow visibility、VPL samples。
2. 以点云/mesh-like scatter 展示 G-Buffer 空间结构。
3. 显示 VPL 采样点、area light plane、point light cube faces、camera frustum。
4. 支持从 manifest 选择 scene，并和当前 2D heatmap/contact sheet 联动。
5. 优先用轻量技术栈：`open3d` 或 `plotly`/`pyvista` 二选一；如果要浏览器式交互再考虑 `viser`/Three.js 导出。
