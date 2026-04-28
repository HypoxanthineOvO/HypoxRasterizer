# M3: G-Buffer, Depth, and Float Output

## 需求

把当前临时 PNG 输出扩展为清晰、可验证、可扩展的 buffer 系统。G-Buffer 必须支持 float buffer 输出，PNG 只作为预览或 debug；设计要为后续定点量化预留 format metadata 和 writer 接口。

必须支持的通道：

- world position
- view position
- linear depth
- world normal
- view normal
- albedo/baseColor
- materialId
- roughness
- metallic
- shadow visibility

实施计划：

1. 引入清晰的 framebuffer/render target/GBuffer 数据结构，避免继续把所有 buffer 管理散落在 `Rasterizer` 中。
2. 定义 buffer channel 枚举、数据格式、维度、坐标空间和可视化策略。
3. 实现 float buffer dump writer，同时保留 PNG preview writer；如果选择具体 float 文件格式，优先选简单、易验证、无重依赖的方案，并保留替换空间。
4. 明确 depth 的定义：至少包含 linear depth，避免当前 depth normalization 的除零问题。
5. 在 fragment 阶段写入 position、normal、UV/material-derived 参数和 visibility 等数据。
6. 支持按 CLI/schema 选择输出哪些 buffer。
7. 为定点量化预留接口：channel metadata、range、scale/offset 或后续 quantizer 插入点；本里程碑不实现具体位宽量化。

约束：

- 坐标空间必须在代码和文档中明确，不允许只凭变量名猜测。
- G-Buffer 的内部数据以 float 为主，PNG preview 只能是派生输出。
- 不在本里程碑做完整 PBR shading，但要让 roughness/metallic 等材质参数能进入 G-Buffer。

## 预期测试

- 小三角形或平面场景验证 position、normal、linear depth 的数值范围和坐标空间。
- 空场景、全背景、低分辨率、单像素尺寸不崩溃。
- float dump 和 PNG preview 均按 `outputs/<scene-name>/gbuffer/` 生成。
- 关闭/开启指定 buffer 输出时行为正确。
- depth normalization 不再因为 min/max 相等而产生 NaN 或崩溃。

## 预期产出

- GBuffer / framebuffer 数据结构。
- buffer channel enum 和 metadata。
- float buffer writer 与 PNG preview writer。
- 更新后的输出示例和 README 说明。
- `.pipeline/architecture.md` 中记录 buffer contract、坐标空间和量化预留点。
