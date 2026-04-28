# M15: Interactive 3D Viewer

## 需求

在静态 3D 预览可用后，提供可选交互式 3D viewer，便于人工检查 mesh、lights、VPL samples、camera 和 bounds。交互 viewer 不能成为 headless 测试阻塞项。

实施计划：

1. 新增 `hypo_viz.viewer3d` CLI，支持打开 V2 scene config 和已渲染 output root。
2. 提供开关：mesh、bounds、camera frustum、lights、VPL points、world_position point cloud。
3. 支持点大小、VPL 颜色、mesh opacity、bounds-only 等参数。
4. GUI 不可用时 fallback 到 M14 static render 或输出明确 unavailable message。
5. 设计 viewer state JSON，便于测试和复现实验设置。

约束：

- 单元测试不能依赖人工交互。
- 默认命令在 headless 环境下必须可预测。
- viewer 作为实验工具，不影响 renderer 运行。

## 预期测试

- CLI 参数解析和 viewer state 生成测试。
- headless 环境 fallback 测试。
- AreaVPL scene 的 viewer state 包含 mesh/lights/VPL layers。

## 预期产出

- `tools/python/hypo_viz/viewer3d.py`
- `tools/python/tests/test_viewer3d.py`
- `docs/3d-visualization.md` viewer 使用说明
- `.pipeline/architecture.md` 追加 M15 执行记录
