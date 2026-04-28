# M16: G-Buffer, Shadow, and VPL 3D Linkage

## 需求

把现有 renderer 输出和 3D 可视化联动起来：`world_position.f32` 转点云、shadow map face 与 light/frustum 标注、VPL samples 同时出现在 2D overlay 和 3D glyph 中，并生成最终 3D visualization report。

实施计划：

1. 读取 `gbuffer/world_position.f32`、`world_normal.f32`、`shadow_visibility.f32` 生成可降采样点云。
2. 为 Point light cube faces 标注六个方向，输出 shadow face metadata。
3. 为 Directional/Area shadow map 生成 3D light/frustum 标记。
4. 新增一条 scene report 命令，同时输出 2D heatmap/contact sheet 和 3D snapshot。
5. 将 AreaVPL 对比场景接入 3D 报告，显示 VPL sample count、shadow on/off、mean stats。

约束：

- 点云默认需要 stride/downsample，不能直接把大 buffer 全量画爆。
- 3D 报告必须能在 headless/static 模式完成。
- 不把大型生成图纳入 git，只写入 `build/test_outputs` 或 report 指向。

## 预期测试

- world_position `.f32` 转点云并降采样。
- AreaVPL scene 一条命令生成 2D overlay、3D VPL scatter、shadow contact sheet。
- Shadow cube face labels 和 3D face directions 一致。
- 最终 report 包含产物路径和 warning summary。

## 预期产出

- `tools/python/hypo_viz/report3d.py`
- `tools/python/tests/test_report3d.py`
- `build/test_outputs/3d_viz_report/`
- `reports/3d-visualization-*.md`
- `.pipeline/architecture.md` 追加 M16 执行记录
