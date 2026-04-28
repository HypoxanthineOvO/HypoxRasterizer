# HypoxRasterizer - 开发进度

> 最后更新：2026-04-28 16:03（上海时间） | 状态：Release 准备完成 | 进度：20/20 Milestone

## 当前状态

✅ **Release v0.1.0 准备完成** - 已清理本地生成物，补充版本文件和 changelog，发布回归通过；尚未执行 commit/tag/push。

## Milestone 进度

| # | Milestone | 状态 | 摘要 |
|---|-----------|------|------|
| M1 | Build, Test, and Baseline Recovery | ✅ 完成 | 构建、baseline tests、smoke render 已恢复 |
| M2 | Scene Schema, CLI, and Output Contract | ✅ 完成 | 新 schema、CLI 和输出目录契约 |
| M3 | G-Buffer, Depth, and Float Output | ✅ 完成 | Float G-Buffer、Depth 和 PNG preview |
| M4 | Material System and Shading Correctness | ✅ 完成 | 材质系统修正与主流通道扩展 |
| M5 | Directional/Point Lights and Cube Shadow Maps | ✅ 完成 | Directional/Point 光源和 Cube Shadow |
| M6 | VPL Area Lighting, Demo Scenes, and Quality Pass | ✅ 完成 | VPL 展示场景和质量收敛 |
| M7 | Validation Dataset Manifest and Runner Foundation | ✅ 完成 | validation manifest 与 runner |
| M8 | Fast Corner Cases and Standard Internal Scenes | ✅ 完成 | fast corner cases 与 standard 内部场景 |
| M9 | External GI Scene Source Ingestion | ✅ 完成 | 外部 GI 场景下载、许可、转换 hook |
| M10 | Output Statistics, Hashes, and Tolerance Baselines | ✅ 完成 | 输出统计、hash、容差基线 |
| M11 | uv-managed 2D Visualization Tools | ✅ 完成 | Python/uv 2D 可视化 |
| M12 | Heavy Scene Validation and Documentation Pass | ✅ 完成 | heavy 验证、性能预算、文档收束 |
| M13 | Python 3D Scene Data Model | ✅ 完成 | V2 scene、mesh、camera、light、VPL/debug buffer 的 Python scene graph |
| M14 | Static 3D Preview and Headless Screenshots | ✅ 完成 | headless/static 3D PNG、mesh/bounds、light glyph、VPL points |
| M15 | Interactive 3D Viewer | ✅ 完成 | 可选交互 viewer，支持 layer toggles 和 headless fallback |
| M16 | G-Buffer, Shadow, and VPL 3D Linkage | ✅ 完成 | world_position 点云、shadow face/frustum、VPL 2D/3D 联动 |
| M17 | IndirectVPL Design and Data Contract | ✅ 完成 | IndirectVPL schema、sample struct、`.f32`/metadata 输出契约 |
| M18 | IndirectVPL First-Bounce Sampler | ✅ 完成 | Area/Point/Directional first-bounce 间接 VPL 采样 |
| M19 | IndirectVPL Shading and Visibility Integration | ✅ 完成 | 间接光 gather、visibility/shadow 开关、debug channel |
| M20 | IndirectVPL Validation and Visualization | ✅ 完成 | Cornell/texture/Dragon 对比、2D/3D overlay 和最终报告 |

## 最近活动

- **16:03（上海时间）** Release v0.1.0 准备完成：清理生成物、更新 `.gitignore`、新增 `VERSION`/`CHANGELOG.md`，发布回归通过。
- **14:55（上海时间）** Pipeline 完成：20/20 Milestone 全部完成，最终状态写入 `.pipeline/state.yaml`。
- **14:55（上海时间）** M20 完成：IndirectVPL validation runner、Python 3D report、核心 C++/Python 回归全部通过。
- **14:53（上海时间）** M20 implement 完成：新增 Cornell/Money/Dragon IndirectVPL 场景、manifest 注册、direct-only 对比、2D/3D 可视化报告和最终文档。
- **14:50（上海时间）** M20 run_tests_red 完成：缺失 IndirectVPL validation scene 与 visualization/report 支持，符合预期。
- **14:49（上海时间）** M20 write_tests/review_tests 完成：覆盖 IndirectVPL validation 注册、2D VPL overlay 和 3D report linkage。
- **14:46（上海时间）** M19 完成：renderer 接入 IndirectVPL 采样、gather、visibility/clamp 和 debug 输出；IndirectVPLTests 10/10 通过。
- **14:44（上海时间）** M18 完成：新增 first-bounce IndirectVPL sampler、ray/triangle 求交和确定性采样；IndirectVPLTests 8/8 通过。
- **14:36（上海时间）** M17 完成：新增 IndirectVPL config、sample record、float buffer contract 和 C++ 测试；相关 C++ 回归通过。
- **14:23（上海时间）** M16 完成：新增 `hypo_viz.report3d`、`hypo-viz-3d-report`，生成 AreaVPL 2D/3D 联动报告；34 个 Python 测试通过。
- **14:13（上海时间）** M15 完成：新增 `hypo_viz.viewer3d`、`hypo-viz-3d`、HTML viewer state 和 headless fallback；30 个 Python 测试通过。
- **14:05（上海时间）** M14 完成：新增 `hypo_viz.render_3d_static`、`hypo-viz-3d-static`，生成 AreaVPL/Dragon 静态 3D 示例；26 个 Python 测试通过。
- **13:56（上海时间）** M13 完成：新增 `hypo_viz.scene3d`、scene graph 测试和 3D 可视化文档；22 个 Python 工具测试通过。
- **13:48（上海时间）** 用户确认追加规划可执行；Pipeline 从 M13 `write_tests` 开始。
- **13:42（上海时间）** IndirectVPL 追加规划完成：新增 M17-M20，草案写入 `reports/indirect-vpl-plan-draft-2026-04-28.md`，确认摘要更新为 `.pipeline/confirm-summary-3d-viz.md`。
- **13:35（上海时间）** 高清 validation 渲染与 VPL/Shadow 对比报告完成：写入 `reports/highres-validation-2026-04-28.md`，输出位于 `build/highres_validation_20260428/`。
- **13:28（上海时间）** 高清 validation 渲染完成：10 个可运行场景以 512x512 输出，2 个预期缺失资源场景 skip；报告写入 `.pipeline/reports/12-highres-validation-render-report.md`。
- **13:28（上海时间）** Python 3D 可视化规划 P1 Discover 草稿已写入 `.plan-state/3d-viz-discover.yaml`，等待用户确认方向。
- **13:19（上海时间）** Pipeline 完成：12/12 Milestone 全部完成，最终报告写入 `.pipeline/reports/11-heavy-scene-validation-docs.md`。
- **13:19（上海时间）** M12 完成：heavy Dragon low-res benchmark 通过，Bistro external hook 缺失时 clean skip，perf metadata 已记录。
- **13:14（上海时间）** M12 run_tests_red 完成：测试因 heavy Dragon config、external hook、perf metadata 和最终文档缺失失败，符合预期。
- **13:13（上海时间）** M12 write_tests/review_tests 完成：新增 heavy manifest、heavy render pass/skip、perf metadata 和文档命令测试。
- **13:10（上海时间）** M12 启动，进入 write_tests；M11 完成，报告写入 `.pipeline/reports/10-uv-2d-visualization-tools.md`。
- **13:10（上海时间）** M11 完成：实现 heatmap、VPL overlay、shadow contact sheet、CLI、文档和 14 个示例 PNG。
- **13:07（上海时间）** M11 run_tests_red 完成：测试因 `hypo_viz.render_2d` 缺失失败，符合预期。
- **13:06（上海时间）** M11 write_tests/review_tests 完成：新增 heatmap、VPL overlay、shadow contact sheet、warning 和稳定命名测试。
- **13:04（上海时间）** M11 启动，进入 write_tests；M10 完成，报告写入 `.pipeline/reports/09-output-baseline-stats.md`。
- **13:04（上海时间）** M10 完成：实现 `.f32`/PNG stats、baseline JSON、容差/strict hash 比较，并生成 fast/VPL baseline。
- **13:00（上海时间）** M10 run_tests_red 完成：测试因 baseline collector/comparator 模块缺失失败，符合预期。
- **12:59（上海时间）** M10 write_tests/review_tests 完成：新增 `.f32`/PNG stats、baseline comparator 和输出树 roundtrip 测试。
- **12:56（上海时间）** M10 启动，进入 write_tests；M9 完成，报告写入 `.pipeline/reports/08-external-gi-scene-ingestion.md`。
- **12:56（上海时间）** M9 review_code 完成：清理生成物残留，补 `.gitignore`，确认 external source 文档和 skip 语义。
- **12:55（上海时间）** M9 run_tests_green 完成：uv Python 单测、fetcher dry-run、heavy skip、py_compile 和 validation C++ 测试通过。
- **12:55（上海时间）** M9 implement 完成：新增 uv Python 工具项目、external source manifest/fetcher、文档和 runner skip 语义。
- **12:43（上海时间）** M9 run_tests_red 完成：测试因 `hypo_assets` 外部资源工具尚未实现失败，符合预期。
- **12:42（上海时间）** M9 write_tests/review_tests 完成：新增 Python unittest 覆盖 external source 校验、dry-run、代理端口和 optional/heavy skip。
- **12:38（上海时间）** M9 启动，进入 write_tests；M8 完成，报告写入 `.pipeline/reports/07-fast-standard-validation-scenes.md`。
- **12:38（上海时间）** M8 review_code 完成：补 texture path 断言，更新 validation 文档和 architecture 记录。
- **12:36（上海时间）** M8 run_tests_green 完成：C++ 回归、runner dry-run、fast 默认真实渲染、完整 standard 真实渲染全部通过。
- **12:33（上海时间）** M8 implement 完成：新增 fast corner case、standard texture/Bunny/Dragon quick 配置，注册 manifest，并补 V2 texture adapter。
- **12:28（上海时间）** M8 run_tests_red 完成：ValidationSuiteTests 因 fast/standard validation scene entries 缺失失败，符合预期。
- **12:27（上海时间）** M8 review_tests 完成：ValidationSuiteTests 覆盖 fast corner cases、纹理、Bunny、Dragon quick 和 missing asset 诊断，进入 RED。
- **12:27（上海时间）** M8 write_tests 完成：新增 ValidationSuiteTests 并注册 xmake target。
- **12:24（上海时间）** M7 完成，报告写入 `.pipeline/reports/06-validation-dataset-runner.md`；自动进入 M8 write_tests。
- **12:24（上海时间）** M7 review_code 完成：修正 runner 显式 heavy 枚举语义。
- **12:22（上海时间）** M7 run_tests_green 完成：HypoxRasterizer 构建、七组测试和 validation runner dry-run 全部通过。
- **12:22（上海时间）** M7 implement 完成：实现 validation manifest parser、选择器、引用诊断、仓库 manifest、dry-run runner 和文档。
- **12:18（上海时间）** M7 run_tests_red 完成：ValidationSceneTests 因 `validation_manifest.hpp` 缺失失败，符合预期。
- **12:16（上海时间）** M7 review_tests 完成：ValidationSceneTests 覆盖 manifest 解析、tier/tag 过滤、heavy 排除、缺失资源诊断和 repository dry-run。
- **12:16（上海时间）** M7 write_tests 完成：新增 ValidationSceneTests 并注册 xmake target。
- **12:07（上海时间）** 追加规划 P3/P4 完成：生成 prompts 06-11、更新 architecture/state/confirm summary，等待用户确认执行。
- **12:03（上海时间）** 追加规划 P2 decompose 完成初稿：拟追加 M7-M12，覆盖 validation manifest、fast/standard scenes、external GI ingestion、baseline stats、uv 2D visualization 和 heavy validation。
- **12:03（上海时间）** 追加规划 P1 discover 完成：确认使用 fast/standard/heavy 分层、外部资源下载、7890-7892 代理、Python/uv 2D 可视化、统计/hash/容差基线。
- **22:51（上海时间）** Pipeline 完成：6/6 Milestone 全部完成，最终报告写入 `.pipeline/reports/05-vpl-area-demo-quality.md`。
- **22:51（上海时间）** M6 review_code 完成：最终 diff 审阅无阻塞问题，Cascade Shadow Maps 继续延后。
- **22:50（上海时间）** M6 run_tests_green 完成：HypoxRasterizer 构建、六组测试、Point/Directional/Area 三个 smoke 渲染全部通过。
- **22:50（上海时间）** M6 implement 完成：实现可配置 Area VPL、VPL debug `.f32`、AreaVPL demo 和输出目录清理。
- **22:42（上海时间）** M6 run_tests_red 完成：VPLAreaTests 因 AreaLight 可配置 VPL API 缺失而失败，符合预期。
- **22:41（上海时间）** M6 review_tests 完成：覆盖 VPL 采样数量、seed 可复现性和 V2 area VPL 字段，进入 RED。
- **22:41（上海时间）** M6 write_tests 完成：新增 VPLAreaTests 并注册 xmake target。
- **22:38（上海时间）** M5 完成，报告写入 `.pipeline/reports/04-directional-point-cube-shadow.md`；自动进入 M6 write_tests。
- **22:38（上海时间）** M5 review_code 完成：确认 Cascade 延后，light helper namespace 已清理。
- **22:36（上海时间）** M5 run_tests_green 完成：HypoxRasterizer 构建、五组测试、Point smoke 和 Directional smoke 全部通过。
- **22:36（上海时间）** M5 implement 完成：实现 DirectionalLight、Point cube face 输出命名、directional 配置适配和 shadow_visibility 导出。
- **22:29（上海时间）** M5 run_tests_red 完成：LightShadowTests 因 DirectionalLight 和 Point cube face labels 缺失而失败，符合预期。
- **22:28（上海时间）** M5 review_tests 完成：LightShadowTests 覆盖 Point cube face、Directional 方向归一化和 V2 directional 解析，进入 RED。
- **22:28（上海时间）** M5 write_tests 完成：新增 LightShadowTests 并注册 xmake target。
- **22:24（上海时间）** M4 完成，报告写入 `.pipeline/reports/03-material-system.md`；自动进入 M5 write_tests。
- **22:23（上海时间）** M4 run_tests_green 完成：四组测试全部通过，V2 smoke 保留 roughness/metallic 并生成材质 G-Buffer。
- **22:21（上海时间）** M4 implement 完成：新增 MaterialSample/PBRMaterial，修复纹理尺寸，并接入 roughness/metallic。
- **22:19（上海时间）** M4 run_tests_red 完成：`MaterialSample` / `PBRMaterial` 缺失，失败符合预期。
- **22:18（上海时间）** M4 review_tests 完成：覆盖范围足够，进入 RED。
- **22:18（上海时间）** M4 write_tests 完成：新增 MaterialTests，覆盖采样、PBR 参数、纹理尺寸和 V2 材质字段。
- **22:15（上海时间）** M3 完成，报告写入 `.pipeline/reports/02-gbuffer-depth-float-output.md`；自动进入 M4 write_tests。
- **22:14（上海时间）** M3 run_tests_green 完成：GBufferTests 4/4，V2 smoke 生成 `.f32` G-Buffer 和 depth preview。
- **22:12（上海时间）** M3 implement 完成：新增 GBuffer、FloatBufferWriter，并导出 `.f32` G-Buffer 通道。
- **22:09（上海时间）** M3 run_tests_red 完成：`float_buffer_writer.hpp` 缺失导致编译失败，符合预期。
- **22:07（上海时间）** M3 review_tests 完成：覆盖范围足够，进入 RED。
- **22:06（上海时间）** M3 write_tests 完成：新增 GBufferTests，覆盖通道、depth preview 和 float writer。
- **22:00（上海时间）** M2 完成，报告写入 `.pipeline/reports/01-scene-cli-output-contract.md`；自动进入 M3 write_tests。
- **21:55（上海时间）** M2 run_tests_green 完成：BaselineTests 6/6、SceneCliOutputTests 6/6、V2 smoke 输出目录验证通过。
- **21:53（上海时间）** M2 implement 完成：新增 CLI、V2 schema loader、OutputManager、V2 临时渲染适配和文档。
- **21:48（上海时间）** M2 run_tests_red 完成：`cli.hpp` 缺失导致编译失败，符合预期。
- **21:47（上海时间）** M2 review_tests 完成：覆盖范围足够，进入 RED。
- **21:47（上海时间）** M2 write_tests 完成：新增 CLI、v2 scene schema 和 output layout 测试。
- **21:45（上海时间）** M1 完成，报告写入 `.pipeline/reports/00-build-test-baseline.md`；自动进入 M2 write_tests。
- **21:44（上海时间）** M1 run_tests_green 完成：构建、baseline tests、smoke render 全部通过。
- **21:43（上海时间）** M1 implement 完成：接入 `BaselineTests`，补充 smoke config，并规避 header-only 依赖下载失败。
- **21:43（上海时间）** `xmake run BaselineTests` 通过：6 passed, 0 failed。
- **21:42（上海时间）** smoke render 通过：生成 color/depth/normal 和 6 张 Point light shadow map。
- **21:35（上海时间）** M1 run_tests_red 完成：`BaselineTests` 尚未注册为 xmake target，失败符合预期。
- **21:34（上海时间）** M1 review_tests 完成：覆盖范围足够，进入 RED。
- **21:34（上海时间）** M1 write_tests 完成：新增 6 个 baseline 检查和低分辨率 smoke config。
- **21:31（上海时间）** M1 启动，进入 write_tests。

## Deferred 项

- 暂无。
