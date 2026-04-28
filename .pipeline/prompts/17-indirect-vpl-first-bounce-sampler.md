# M18: IndirectVPL First-Bounce Sampler

## 需求

实现 first-bounce IndirectVPL 采样器。采样器从场景光源发射有限数量样本，和 mesh 几何求交，在第一个可见表面生成间接光 VPL。

推荐实现顺序：

1. 先建立轻量 geometry query：
   - 复用已加载 object/triangle 数据。
   - 初版可用简单加速结构或分对象 bounds 过滤；如果性能不足再加 BVH。
2. 支持 light emission sampling：
   - Area light：从矩形面采样位置和方向。
   - Point light：从球面方向采样。
   - Directional light：从 scene bounds 上方采样平行射线。
3. 首次交点生成 VPL：
   - position 略微沿 normal 偏移，避免自相交。
   - normal/material/albedo 来自命中三角形或当前材质。
   - flux 根据光源强度、采样 pdf、入射角和材质反射率估算。
4. 保证 seed 可复现、sample count 可控、无命中样本可统计。
5. 输出 `.f32` sample buffer 和 metadata JSON。

约束：

- 初版只承诺 one-bounce。
- 先保证 Cornell/Dragon quick 可在合理时间内跑完。
- 对外部 heavy scene 默认允许 lower sample count 或 bounds-only fallback。

## 预期测试

- Cornell/AreaVPL 场景在固定 seed 下生成稳定数量与稳定 sample buffer。
- Point/Directional light 各自至少有一个 smoke scene 能产生 IndirectVPL。
- 离屏/空场景/缺失 geometry 返回清晰 warning 或零样本，不崩溃。
- Dragon quick 在低 sample count 下通过 runtime budget。

## 预期产出

- IndirectVPL sampler 实现。
- `Tests/IndirectVPLTests.cpp` 扩展 sampler 测试。
- 新增 `configs/validation/standard/*indirect_vpl*.v2.json`。
- `.pipeline/reports/17-indirect-vpl-first-bounce-sampler.md`。
