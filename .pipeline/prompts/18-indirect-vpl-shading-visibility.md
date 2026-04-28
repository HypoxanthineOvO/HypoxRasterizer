# M19: IndirectVPL Shading and Visibility Integration

## 需求

把 IndirectVPL gather 接入主 shading 路径，支持 direct-only、direct+IndirectVPL、visibility on/off、shadow on/off 的对比实验。

实施计划：

1. 在 G-Buffer/fragment shading 阶段为每个可见点累加 IndirectVPL 贡献。
2. 初版 BRDF 采用保守 diffuse 项，并保留 roughness/metallic 对后续 PBR 的数据通道。
3. 对 VPL 几何项做距离衰减、normal dot、radius/clamp，避免近距离 singularity。
4. 支持 visibility 查询开关：
   - `visibility=false`：纯 gather，用于看样本分布和能量。
   - `visibility=true`：做 shadow/ray visibility 或复用可行的 shadow query。
5. 输出调试通道：
   - `gbuffer/indirect_vpl_radiance.f32`
   - `gbuffer/indirect_vpl_count.f32` 或 metadata stats。
6. 加 CLI/config 对照入口，便于自动生成比较图。

约束：

- 默认不开启，保持既有 baseline 不漂移。
- 视觉正确性和数据可解释性优先，性能只要求不要离谱。
- 必须能在 validation runner 中用较低 sample count 做自动回归。

## 预期测试

- 开启 IndirectVPL 后 color 输出相对 direct-only 有可测差异。
- visibility on/off 会改变结果，并写出不同 metadata。
- clamp/radius 能防止异常高亮样本。
- 所有旧测试在默认关闭时继续通过。

## 预期产出

- Renderer shading 集成。
- IndirectVPL debug G-Buffer / metadata。
- 对比用 CLI/config 示例。
- `.pipeline/reports/18-indirect-vpl-shading-visibility.md`。
