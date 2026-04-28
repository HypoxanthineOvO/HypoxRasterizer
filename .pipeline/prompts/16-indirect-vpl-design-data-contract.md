# M17: IndirectVPL Design and Data Contract

## 需求

在现有 direct VPL / shadow / G-Buffer 基础上追加 `IndirectVPL` 间接光采样设计。这个 milestone 只定义稳定契约和测试，不急着把采样器塞进主渲染循环。

默认方案采用 first-bounce VPL：从 light 采样射线，和场景求交，在首个受光表面生成一个带位置、法线、材质和通量的 VPL；主相机命中点再从这些 VPL gather 间接光。

实施计划：

1. 设计 V2 scene schema 字段，例如 `indirectVPL.{enabled,samples,seed,bounces,visibility,clamp,radius,sourceLights}`。
2. 定义 C++ 数据结构 `IndirectVPLSample`：`position`、`normal`、`flux/radiance`、`radius`、`bounceIndex`、`sourceLightIndex`、`flags`。
3. 区分 direct area VPL 和 indirect VPL 输出命名：
   - direct area: `shadow/light_<index>_direct_vpls.f32`
   - indirect first bounce: `shadow/light_<index>_indirect_vpls.f32`
   - metadata: `shadow/light_<index>_indirect_vpls.json`
4. 明确默认只做 one-bounce diffuse/glossy-lite 间接光；RSM、多反弹、MIS、能量校准作为后续扩展。
5. 写 schema/config/output contract 测试。

约束：

- 保持默认关闭，避免改变现有 validation baseline。
- 需要能和 shadow on/off、direct VPL sample count 一起做对照实验。
- 输出仍以 `.f32` 作为 authoritative buffer，PNG 仅用于可视化。

## 预期测试

- V2 config 能解析 `indirectVPL` 字段和默认值。
- legacy adapter 不开启 `indirectVPL` 时行为完全不变。
- `IndirectVPLSample` float dump header/shape/metadata 稳定。
- 命名不会和现有 direct area VPL 输出冲突。

## 预期产出

- `Utils/Configs` 中的 IndirectVPL config 字段。
- `Modules/Light` 或独立 lighting/debug 模块中的 IndirectVPL 数据结构。
- `Tests/IndirectVPLTests.cpp`
- `docs/scene-config-v2.md` 更新。
- `.pipeline/architecture.md` 追加 M17 执行记录。
