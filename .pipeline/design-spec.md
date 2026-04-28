# Design Spec

## Goal

完善已有的 C++ 软件光栅器，使其从课程/科研原型演进为可维护、可验证、可扩展的离线渲染工具。核心目标是稳定读取场景，输出最终渲染图、G-Buffer、Depth Map、Shadow Map 等科研/调试所需图像，并逐步支持主流材质、光源和常见场景结构。

## Project Shape

- Project type: C++17 CPU software rasterizer / research renderer.
- Primary deliverable: 可运行的命令行渲染器、可复用渲染模块、场景配置、输出图像与调试缓冲。
- Target platform: 当前 Linux/xmake 环境优先。
- Expected users: 项目作者本人，以及需要可控 G-Buffer / Shadow Map / 材质输出的科研或实验使用者。

## Constraints

- Tech stack constraints:
  - 保留 C++17、xmake、Eigen、stb、nlohmann_json、tinyobjloader 的现有技术栈，除非后续明确需要替换。
  - 优先修复和扩展当前 CPU rasterizer，不先切换到 OpenGL/Vulkan/Embree 等渲染后端。
- Tooling constraints:
  - 现有 `Tests/` 只是独立测试源码，`xmake.lua` 中测试 target 被注释；规划中需要先恢复可执行验证入口。
  - 规划模式为 interactive，需要用户在关键 checkpoint 确认。
- Compatibility constraints:
  - 尽量保留现有 JSON 场景配置和 assets 目录可用。
  - 输出文件名和目录结构可改进，但需要有兼容或迁移说明。

## Existing Context

- Existing repo detected: yes.
- Existing `.pipeline/` detected: yes, initialized before planning.
- Current architecture baseline: `.pipeline/architecture/`.
- Key directories:
  - `Rasterizer/`: 主渲染管线。
  - `Modules/Object/`: OBJ、Triangle、Material、Scene。
  - `Modules/Camera/`: camera/view/projection。
  - `Modules/Light/`: Light 和 ShadowMap。
  - `Utils/`: 配置、图像 IO、数学工具。
  - `configs/`, `assets/`: 现有测试场景与模型资产。

## Functional Requirements

- Core capabilities:
  - 稳定渲染最终 color image。
  - 输出 G-Buffer：包括 world position、view position、linear depth、world normal、view normal、albedo/baseColor、materialId、roughness、metallic、shadow visibility 等通道。
  - G-Buffer 需要保留 float buffer 输出能力，PNG 只作为预览或 debug 输出；buffer 设计要为后续定点量化留出空间。
  - 输出 Depth Map，避免当前深度归一化边界问题。
  - 输出不同光源对应的 Shadow Map，并支持更清晰的 debug 命名和目录组织。
  - 先修正并稳定现有材质/着色模型，再继续扩展主流材质参数与纹理通道。
  - 材质路线初定为：现有 Blinn-Phong / Phong 兼容层优先，后续支持 PBR metallic-roughness、albedo/normal/roughness/metallic/AO 等贴图通道。
  - 光源路线初定为：Directional 和 Point 必须优先支持；Point 需要 Cube Shadow Map；Area 后续通过 VPL 采样支持；Cascade Shadow Map 暂不作为第一批必做，后续结合 directional light 质量需求讨论。
  - 改进场景/配置结构，使未来添加 mesh、material、light、camera、outputs 更直接。
  - VPL 采样和 Cube Shadow 是重点展示能力，应有足够多的示例和可视化输出，而不是只做最小实现。
- Command surface:
  - 当前入口为 `./HypoxRasterizer <config.json>`。
  - 后续需要支持命令行参数，至少包括输出目录、输出 buffer 选择、分辨率覆盖、debug/benchmark 参数。
  - 输出目录初定为 `outputs/<scene-name>/`，其中包含 `color.png`、`gbuffer/`、`shadow/` 等子输出。
- Review / reporting needs:
  - 每个里程碑需要有明确构建/测试/渲染验证点。

## Testing Expectations

- Required validation strategy:
  - 先恢复 xmake 下可执行的基础测试 target。
  - 为几何、相机矩阵、配置解析、图像输出、深度/插值、shadow visibility 加基本回归测试。
  - 对渲染输出采用 smoke test、小分辨率 deterministic 场景验证、float buffer 数值检查和 PNG 预览检查组合。
- Boundary / failure cases:
  - 缺失 JSON 字段、非法材质/光源类型、缺失 OBJ/贴图、无法归一化的深度范围。
  - 三角形退化、屏幕外 AABB、空场景、无光源、多光源。
- Backward compatibility requirements:
  - 可以重新设计 scene schema；现有 `configs/*.json` 不需要作为长期格式约束，但规划中应考虑迁移或示例重写。

## Milestone Strategy

- Proposed milestone count: 6.
- Milestone split rationale:
  - 先建立可验证基础，再重构输出/配置，再扩展材质和光源，最后做质量提升与示例场景。
- Expected preset: `tdd`，因为这是工程型改造且需要避免渲染管线回归。
- Priority order:
  - 架构结构简单明了优先。
  - 数据准确性和视觉效果并重。
  - 性能不作为第一优先级，但避免明显不合理的复杂度或数量级退化。
- Proposed milestones:
  - M1: 构建、测试和基线验证恢复。
  - M2: 新 scene schema、CLI 和输出目录结构。
  - M3: G-Buffer / Depth / Float Buffer 输出基础。
  - M4: 材质系统修正与主流材质参数扩展。
  - M5: Directional / Point 光源与 Cube Shadow Map。
  - M6: Area Light VPL、示例场景和质量收敛。

## Open Questions

- 输出文件组织与命令行接口：是否需要统一输出目录、可选 buffer 列表、分辨率覆盖、批量场景渲染？
- 新 scene schema 的最小结构和可扩展边界需要在里程碑中设计。
- 定点量化的具体格式、位宽和误差度量后续单独设计；当前只要求 buffer API 和文件组织不阻碍扩展。
- Demo / 验收场景需要覆盖 VPL 采样、Cube Shadow、多模型、多材质、多光源和基础 Cornell/Bunny/Dragon 等代表性场景。

## Notes

- 用户背景：这是去年无 AI Agent 时完成的科研项目原型，已有基础能力但未完成到满意水平。
- 用户目标：完善为能输出图片、G-Buffer、不同类型光源 Shadow Map、Depth Map，并支持主流材质和结构的光栅器。

## Append Plan: Validation Scene Data Expansion

### New Goal

在第一轮渲染能力打通之后，追加建设一套系统化测试场景数据，用于验证最终图片、G-Buffer、Depth、Shadow Map、Cube Shadow Map、Area VPL debug 等输出。目标不是只增加几个 demo，而是形成可复现的 validation scene suite，让后续材质、光照、shadow、场景结构变化都有稳定回归依据。

### Current Gap

- 现有 V2 场景主要是 smoke 级：`SmokeTest.v2.json`、`DirectionalSmoke.v2.json`、`AreaVPLSmoke.v2.json`。
- 仓库已有 CornellBox、Bunny、Dragon、Money、Nylon、基础几何等资产，但很多仍只通过 legacy config 使用，缺少统一 V2 验证场景组织。
- 当前测试重点在 API/格式和低分辨率 smoke，缺少场景 manifest、资产健康检查、输出统计 baseline、批量渲染/验证入口。
- 缺少明确标注“这个场景验证什么”的数据集，例如：多材质、纹理、复杂 mesh、point cube shadow 六面覆盖、directional shadow、area VPL 多采样、空/坏资源错误场景。

### Desired Validation Dataset Properties

- 场景数据应分层：fast smoke、standard validation、optional heavy scenes。
- 每个场景应有 tags、资产列表、光源类型、预期输出、运行成本和验证重点。
- 验证方式需要组合：输出文件/header 检查、float buffer 统计、PNG/hash、容差比较。图像肉眼检查只能作为辅助。
- 默认验证需要覆盖纹理场景；Dragon 级别的场景也需要能以较低分辨率或简化配置较快跑好，不能只作为不可执行的大场景摆设。
- Heavy 场景用于专项验证和 benchmark，但 standard 层也要覆盖经典场景、多材质、多光源、纹理和一定复杂度 mesh。
- 允许通过脚本下载外部经典/GI 场景资源；网络下载可使用 7890-7892 端口代理。
- 外部资源需要记录 URL、license、hash、下载/转换方式，避免无来源资产混入仓库。
- Python 实验工具使用 uv 管理，第一轮先做 2D 可视化：读取 `.f32`、叠加 VPL 采样点、Shadow Map face 标记、G-Buffer/depth heatmap 和 annotation PNG。

### Candidate External Scene Sources

- Benedikt Bitterli Rendering Resources：适合 GI/离线渲染参考场景筛选，包含多类经典场景并列出 license。
- PBRT v4 scenes：适合作为物理渲染参考数据和复杂材质/场景结构来源。
- Morgan McGuire / NVIDIA ORCA scenes：适合 Sponza、San Miguel、Bistro 等经典中重型场景参考；需要确认格式转换和许可。

## Append Plan: Python 3D Visualization

### New Goal

在已有 uv 管理的 2D 可视化基础上，规划下一轮 3D 实验可视化。目标是把 `.f32` G-Buffer、VPL sample、Shadow Map、camera/light/debug metadata 组合成可以检查空间关系的 3D 视图，帮助分析 VPL 分布、光源位置、shadow 投影、world position/normal/depth 的一致性。

### Current 2D Baseline

- 已有 `hypo_viz.render_2d`：
  - `.f32` heatmap
  - VPL overlay
  - shadow contact sheet
  - scene batch visualization
- 已有 `hypo_validation.collect_stats`：
  - `.f32` header/payload reader
  - PNG shape/hash/stat baseline
- 已有 validation manifest：
  - fast/standard/heavy tiers
  - output roots and baseline metadata

### 3D Visualization Candidate Capabilities

- 读取 `world_position.f32` 生成点云或稀疏采样点集。
- 使用 `world_normal.f32` 显示 normal glyph 或 normal-colored point cloud。
- 叠加 `light_0_direct_vpls.f32` 中的 VPL sample positions。
- 显示 camera frustum、area light plane、point light position、directional light direction。
- 对 shadow map / shadow visibility 提供空间筛选：在 3D 点云中按 visibility 着色。
- 支持 scene manifest 选择和 batch 输出 HTML/PNG。

### Open Questions For Discover

- 第一版更适合做离线 HTML/浏览器视图，还是本地交互窗口？
- 3D 技术栈优先选择 `plotly`、`open3d`、`pyvista`、还是导出 Three.js/HTML？
- VPL sample 的 6-channel 当前格式需要正式文档化：哪些 channel 表示 position，哪些表示 normal/weight/debug value？
