# M13: Python 3D Scene Data Model

## 需求

在现有 `tools/python` uv 项目中建立 3D 可视化的数据模型。目标是把 V2 scene config、OBJ mesh、对象 transform、camera、lights、VPL/debug `.f32` buffer 统一为 Python scene graph，供静态 3D 渲染和交互 viewer 复用。

实施计划：

1. 在 `tools/python/pyproject.toml` 增加必要依赖，优先考虑 `trimesh`，只在确实需要时加入更重的 3D 渲染库。
2. 新增 `hypo_viz.scene3d` 或等价模块，负责加载 V2 scene config。
3. 解析 object mesh path、material id、translation/rotation/scale、world bounds、triangle/vertex count。
4. 解析 camera position/target/FOV 和 light position/direction/area size/VPL config。
5. 复用 M10 `.f32` reader 读取 `shadow/light_*_direct_vpls.f32`，暴露 VPL 点云数据。
6. 对 heavy mesh 支持 `metadata_only` / `bounds_only` 模式，避免默认加载完整高面数网格。

约束：

- 不修改 C++ renderer 主流程。
- Python 模块必须可在 headless 环境下运行。
- 外部资产缺失时返回结构化 warning，不阻塞其他 scene。

## 预期测试

- 加载 `configs/AreaVPLSmoke.v2.json`，得到 7 个对象、area light、VPL 配置。
- 加载 `configs/validation/standard/bunny_mesh.v2.json`，得到 Bunny mesh metadata。
- 加载 `configs/validation/standard/dragon_quick.v2.json` 的 bounds-only 模式，不强制完整渲染。
- VPL `.f32` buffer 可作为 3D sample positions 载入。

## 预期产出

- `tools/python/hypo_viz/scene3d.py`
- `tools/python/tests/test_scene3d.py`
- `docs/3d-visualization.md`
- `.pipeline/architecture.md` 追加 M13 执行记录
