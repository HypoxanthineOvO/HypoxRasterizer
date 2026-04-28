# IndirectVPL 间接光方案草案

生成时间：2026-04-28 13:42（上海时间）

## 推荐默认方案

第一轮建议做 first-bounce IndirectVPL，而不是一开始做完整路径追踪或 RSM：

1. 从 Area/Point/Directional light 采样光线。
2. 和场景几何求交。
3. 在首个命中表面生成 VPL，记录 position、normal、flux、material、source light、bounce index。
4. 相机可见点从这些 VPL gather 间接光。
5. 用 visibility/shadow 开关做实验对比。

这个方案和当前 CPU rasterizer、G-Buffer、VPL debug buffer、Python 可视化路径最容易接上，也能清楚展示间接光采样点在哪里、它们如何影响最终 color。

## 备选方案

### RSM / Reflective Shadow Map

优点是和 shadow map 体系天然相近，适合从 light view 生成大量 VPL。缺点是 Point light cube RSM、Area light 多视角、纹理/材质采样和遮挡处理会更复杂。建议作为 second pass。

### Camera-visible G-Buffer VPL

实现最快，可直接从当前帧 G-Buffer 拿 visible surface 生成 VPL。缺点是强 view-dependent，不适合作为真实间接光验证。建议只作为 debug fallback。

### 多反弹 VPL

能展示更完整 GI，但能量、噪声、性能和偏差控制都会明显复杂。建议先把 one-bounce 的数据与可视化做稳，再讨论二次反弹。

## 规划 Milestones

| Milestone | 目标 |
|---|---|
| M17 | IndirectVPL 设计与数据契约：schema、sample struct、`.f32`/metadata 输出 |
| M18 | First-bounce 采样器：light emission sampling、geometry hit、flux 估计 |
| M19 | Shading/visibility 集成：gather 间接光、shadow/visibility 开关、debug channel |
| M20 | 验证与可视化：Cornell/texture/Dragon 对比、2D/3D overlay、报告 |

## 需要后续讨论的点

- First-bounce sampler 先用简单 geometry query 还是直接投入 BVH。
- Directional light 的 emission domain 是使用 scene bounds 还是手写 per-scene sampling region。
- IndirectVPL 的能量标定先追求视觉稳定，还是尽早追求物理一致。
- visibility 检查初版做 ray query 还是先允许 shadow-off gather 作为 debug 基线。
