# 3D Python Visualization and IndirectVPL Plan Confirm Summary

生成时间：2026-04-28 13:40（上海时间）
修订时间：2026-04-28 13:42（上海时间）

## 规划目标

在现有 `tools/python` uv 工具链上追加 3D 可视化能力，用于检查 V2 scene、mesh、camera、light、VPL、shadow frustum/cube face，以及 G-Buffer world_position 点云。

同时追加 `IndirectVPL` 间接光采样规划：先实现 first-bounce IndirectVPL，支持 Direct-only / Direct VPL / IndirectVPL / Shadow-visibility on/off 的可视化对照。

## Milestones

| Milestone | Prompt | 目标 |
|---|---|---|
| M13 | `.pipeline/prompts/12-python-3d-scene-data-model.md` | 建立 Python 3D scene graph：V2 scene、mesh metadata、camera、lights、VPL/debug buffers |
| M14 | `.pipeline/prompts/13-static-3d-preview.md` | Headless/static 3D PNG：mesh/bounds、camera frustum、light glyph、area light、VPL points |
| M15 | `.pipeline/prompts/14-interactive-3d-viewer.md` | 可选交互式 3D viewer，headless 环境下 fallback 或明确报错 |
| M16 | `.pipeline/prompts/15-gbuffer-shadow-vpl-3d-linkage.md` | G-Buffer/world_position 点云、shadow face/frustum、VPL 2D/3D 联动报告 |
| M17 | `.pipeline/prompts/16-indirect-vpl-design-data-contract.md` | IndirectVPL schema、sample struct、`.f32`/metadata 输出契约 |
| M18 | `.pipeline/prompts/17-indirect-vpl-first-bounce-sampler.md` | 从 Area/Point/Directional light 采样 first-bounce IndirectVPL |
| M19 | `.pipeline/prompts/18-indirect-vpl-shading-visibility.md` | 把 IndirectVPL gather 接入 shading，支持 visibility/shadow 对照 |
| M20 | `.pipeline/prompts/19-indirect-vpl-validation-visualization.md` | Cornell/texture/Dragon 验证、2D/3D overlay、sample sweep 和最终报告 |

## 推荐技术路线

- `trimesh`：mesh loading、bounds、metadata、transform。
- `PyVista`：主 3D 可视化、glyph、截图、交互窗口。
- `Open3D`：headless/point cloud/camera debug 备选。

官方文档参考：

- https://docs.pyvista.org/
- https://www.open3d.org/docs/latest/tutorial/visualization/index.html
- https://trimesh.org/

## 默认决策

- 第一阶段优先 headless/static 3D screenshot，交互 viewer 后置到 M15。
- Heavy mesh 默认支持 bounds-only，并在后续按需加 decimation 或 LOD config。
- 第一轮 VPL/Shadow 3D 标注先显示世界坐标采样点、light glyph、frustum/cube directions；2D 像素联动放到 M16。
- IndirectVPL 第一轮采用 one-bounce first-hit sampler，默认关闭；RSM、多反弹和 MIS 延后。
- IndirectVPL 输出单独命名为 `light_<index>_indirect_vpls.f32`，不复用 direct area VPL buffer。
- Visibility/shadow 检查必须可开关，便于对比采样分布、遮挡和最终视觉差异。

## 等待确认

确认后将从 M13 开始执行，并保持 TDD 流程：write_tests → review_tests → run_tests_red → implement → run_tests_green → review_code。

如果希望先讨论 IndirectVPL 的具体实现路线，可以在执行 M17 前把采样器方案进一步收窄到：simple geometry query、BVH、或 RSM-first。
