# M14: Static 3D Preview and Headless Screenshots

## 需求

基于 M13 scene graph 生成静态 3D PNG，用于报告和 headless validation。第一版要能展示 mesh/bounds、camera frustum、light glyph、area light rectangle、VPL points，并能在没有 GUI 的环境下给出明确 fallback。

实施计划：

1. 选择主 3D 渲染后端，优先 `PyVista`，保留 `Open3D` 作为 headless fallback 评估项。
2. 实现 `hypo_viz.render_3d_static`，支持 `--config`、`--scene-output-root`、`--output`、`--bounds-only`、`--max-faces`。
3. 为 camera 画 frustum，为 point/directional/area light 画 glyph/arrow/rectangle。
4. 为 VPL samples 画小球或点云，颜色区分 light index。
5. 对 Dragon/Heavy 场景提供 decimated 或 bounds-only 静态预览。
6. 输出 warning/report JSON，记录是否使用 fallback。

约束：

- 静态截图优先于交互窗口。
- 不能要求 CI/服务器必须有显示器。
- 对 heavy mesh 默认不能因为加载太慢阻塞测试。

## 预期测试

- AreaVPL 场景生成一张 3D overview PNG。
- Dragon quick 在 bounds-only 模式生成 PNG。
- 缺失可选 mesh/VPL 时 warning 继续。
- headless 后端不可用时返回清晰错误，不产生半截文件。

## 预期产出

- `tools/python/hypo_viz/render_3d_static.py`
- `tools/python/tests/test_render_3d_static.py`
- `build/test_outputs/3d_viz_static/*.png`
- `docs/3d-visualization.md` 更新
- `.pipeline/architecture.md` 追加 M14 执行记录
