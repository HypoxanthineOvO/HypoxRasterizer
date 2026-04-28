# M11: uv-managed 2D Visualization Tools

## 需求

提供 uv 管理的 Python 2D 可视化工具，用于查看 renderer 输出、VPL 采样位置、Shadow Map、G-Buffer、Depth 和标记图。第一轮只做 2D overlay 和静态图输出，不做 3D 交互。

实施计划：

1. 在统一 `tools/python/pyproject.toml` 中加入 visualization 所需依赖，例如 pillow、numpy、matplotlib、pyyaml。
2. 实现 `.f32` buffer reader 复用 M10 基础能力。
3. 实现 depth/normal/albedo/shadow heatmap 和 contact sheet。
4. 实现 VPL sample overlay：读取 `shadow/light_<index>_direct_vpls.f32`，将采样位置投影或映射到目标 PNG/Map 上，并输出 annotated PNG。
5. 实现 shadow face annotation：为 point cube shadow 六个 face 和 directional/area shadow 输出加标题、face label、basic stats。
6. 支持按 scene output root 和 validation manifest 批量生成可视化结果。
7. 文档中给出命令示例和输出解释。

约束：

- 第一轮不做 OpenGL/Open3D/交互 UI。
- 可视化工具是实验辅助，不应阻塞 renderer 主流程。
- 缺失可选 map 或 VPL debug 文件时应给出 warning 并继续处理其他图。

## 预期测试

- Unit tests 覆盖 `.f32` 解析、heatmap 生成、overlay 输入校验、输出命名稳定性。
- 对 AreaVPL 输出中的 `light_0_direct_vpls.f32` 能生成 annotated PNG。
- 对 point cube shadow 输出能生成 face contact sheet。
- 缺失 shadow/VPL 文件时输出明确 warning。

## 预期产出

- `tools/python/hypo_viz/*.py`
- `uv` 命令入口，如 `uv run hypo-viz ...`
- annotated PNG 示例输出到 `build/test_outputs/...`
- `docs/visualization.md`
- README 增加可视化命令
- `.pipeline/architecture.md` 追加 M11 执行记录
