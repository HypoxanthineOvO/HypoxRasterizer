# Python 3D 可视化规划草案

生成时间：2026-04-28 13:40（上海时间）

## 目标

在现有 `tools/python` 工具链上追加 3D 可视化能力，用于检查：

- V2 scene 中的 mesh、材质、相机、光源位置与方向。
- Point/Directional/Area light 的 shadow frustum 或 cube face 方向。
- VPL 采样点、Area light 采样面、采样密度。
- G-Buffer/world_position 的点云回看。
- validation scene 的 3D 截图或交互查看。

## 推荐技术路线

首选组合：

- `trimesh`：资产读取、OBJ/mesh metadata、包围盒、基础几何处理。
- `PyVista`：主 3D 可视化、mesh/glyph、离屏截图、交互窗口。
- `Open3D`：作为 headless rendering、点云和相机调试的备选路线。

参考官方文档：

- PyVista: https://docs.pyvista.org/
- Open3D visualization: https://www.open3d.org/docs/latest/tutorial/visualization/index.html
- trimesh: https://trimesh.org/

## 建议 Milestones

### M13 3D Scene Data Model

- 读取 V2 scene config。
- 解析 OBJ mesh metadata、bounds、transform。
- 读取 `.f32` VPL/debug buffer。
- 输出统一 Python scene graph。

验证点：

- 能加载 `standard_area_vpl_smoke`、`standard_bunny_mesh`、`standard_dragon_quick`。
- 能列出对象 bounds、材质、相机、光源、VPL 数量。

### M14 Static 3D Preview

- 生成静态 3D PNG。
- 显示 mesh、camera frustum、light glyph、Area light rectangle、VPL point cloud。
- 支持 heavy mesh 降采样或 bounds-only 模式。

验证点：

- AreaVPL 场景生成一张 3D 总览图。
- Dragon 场景可在降采样/bounds-only 模式下生成图。

### M15 Interactive 3D Viewer

- 支持交互窗口打开 scene。
- 支持对象开关、光源开关、VPL 点大小/颜色调整。
- 支持点击或选择对象查看 metadata。

验证点：

- CLI 可启动 viewer。
- 缺失 GUI/headless 环境时输出明确错误或 fallback 到 static render。

### M16 G-Buffer / Shadow / VPL 联动

- world_position `.f32` 转点云。
- shadow map face 与 cube direction 3D 标注。
- VPL overlay 和 3D glyph 同步。
- 输出 3D visualization report。

验证点：

- 选中 AreaVPL scene 可同时生成 2D overlay、3D VPL scatter、shadow contact sheet。

## 关键设计决策

- 3D 可视化是工具链能力，不进入 renderer 主流程。
- 第一阶段必须支持 headless/static 输出，交互窗口作为后续增强。
- heavy/external 场景必须支持降采样、bounds-only 或 lazy loading。
- Python 依赖继续由 `tools/python/pyproject.toml` 和 `uv.lock` 管理。

## 待确认问题

1. 3D viewer 是否优先支持交互窗口，还是优先支持 CI/headless 静态截图？
2. Heavy mesh 默认策略选 mesh decimation、bounds-only，还是按资产手写 LOD config？
3. VPL/Shadow 的 3D 标注是否需要和 2D 图像像素坐标联动，还是第一轮只显示世界坐标中的采样点和 light frustum？
