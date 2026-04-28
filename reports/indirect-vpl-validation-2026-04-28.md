# IndirectVPL Validation Report

生成时间：2026-04-28 14:55（上海时间）

## Summary

本轮完成 first-bounce IndirectVPL 的验证与可视化链路。默认实现不是 RSM 或多反弹 GI，而是从光源发射确定性 first-hit rays，在首个表面生成 IndirectVPL，再由相机可见片元 gather 间接光。

## Validation Scenes

| 场景 | 配置 | 内容 | 重点 |
|---|---|---|---|
| `standard_cornell_indirect_vpl` | `configs/validation/standard/cornell_indirect_vpl.v2.json` | CornellBox、7 个 mesh、Point light、64 个 IndirectVPL | 间接采样、cube shadow、G-Buffer、2D/3D 可视化 |
| `standard_money_indirect_vpl` | `configs/validation/standard/money_indirect_vpl.v2.json` | Money textured mesh、Point light、32 个 IndirectVPL | 纹理资产和 IndirectVPL 验证入口 |
| `heavy_dragon_indirect_vpl_quick` | `configs/validation/heavy/dragon_indirect_vpl_quick.v2.json` | Stanford Dragon quick、Point light、16 个 IndirectVPL | heavy/benchmark 层入口 |

## Main Outputs

- Cornell render: `build/test_outputs/indirect_vpl_validation/standard_cornell_indirect_vpl/color.png`
- Direct-only comparison: `build/test_outputs/indirect_vpl_validation/standard_cornell_direct_only/color.png`
- 2D/3D report: `build/test_outputs/indirect_vpl_report/cornell/report.md`
- 3D overview: `build/test_outputs/indirect_vpl_report/cornell/scene3d.png`
- World-position point cloud: `build/test_outputs/indirect_vpl_report/cornell/world_position_point_cloud.png`
- Indirect VPL overlay: `build/test_outputs/indirect_vpl_report/cornell/viz2d/light_0_indirect_vpls_annotated.png`
- Direct vs indirect contact sheet: `build/test_outputs/indirect_vpl_report/cornell/direct_vs_indirect_contact_sheet.png`
- Direct vs indirect diff: `build/test_outputs/indirect_vpl_report/cornell/direct_vs_indirect_diff.png`

## Comparison

Cornell direct-only vs IndirectVPL:

- Mean absolute RGB diff: `[4.0617, 4.1801, 4.3863]`
- Mean absolute diff: `4.2094`
- Max absolute diff: `46`

这说明当前 IndirectVPL 已经对画面产生可测影响，但仍保持在保守范围内。当前默认 `visibility=false` 用于优先观察采样分布和能量贡献；后续可以在更稳定的场景上逐步开启 visibility。

## Final Verification

- `IndirectVPLTests`: 10 passed
- `ValidationSuiteTests`: 6 passed
- `VPLAreaTests`: 3 passed
- `LightShadowTests`: 5 passed
- `BaselineTests`: 6 passed
- `SceneCliOutputTests`: 6 passed
- `GBufferTests`: 4 passed
- `MaterialTests`: 4 passed
- `ValidationSceneTests`: 5 passed
- Python tools unittest suite: 35 passed
- IndirectVPL standard validation runner: 2 passed, 0 failed

## Known Limits

- 当前是 one-bounce first-hit sampler，不是完整全局光照。
- 采样方向是确定性低成本序列，还没有 BVH、RSM、MIS 或多反弹。
- visibility 查询已经接入，但默认验证场景先关闭，以便避免自遮挡和能量调参混在一起。
- IndirectVPL 能量标定偏工程化，后续需要更严格的能量守恒和 BRDF 匹配。
