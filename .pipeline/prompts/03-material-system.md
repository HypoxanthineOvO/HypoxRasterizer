# M4: Material System and Shading Correctness

## 需求

先把现有材质和着色路径做正确，再扩展主流材质参数和贴图通道。目标不是一次性写出完整电影级 PBR，而是建立结构清晰、数据正确、后续可继续替换 BRDF 的材质系统。

实施计划：

1. 重构 `Materials` 接口，明确 baseColor、roughness、metallic、specular/shininess、emissive、normal map、AO 等参数来源。
2. 修复现有 `TextureMaterial` 对纹理尺寸固定为 1024x1024 的假设，保存实际 width/height/channel 信息。
3. 修复 UV 边界、缺失 texcoord、缺失 texture、采样越界等失败路径。
4. 保留并校准 Blinn-Phong / Phong 兼容着色，使现有场景先能得到合理结果。
5. 加入 PBR metallic-roughness 的数据路径：schema、material class、texture slots、G-Buffer 写入。
6. 初步实现或预留 PBR shading 入口；如果视觉模型先简化，需要在文档里明确当前近似。
7. 支持 albedo/baseColor、normal、roughness、metallic、AO 等贴图通道的配置和采样。

约束：

- 材质参数必须能写入 M3 的 G-Buffer 通道。
- 不把材质解析、纹理采样、BRDF 计算全部塞进 `Rasterizer::FragmentShading`。
- 优先保证结构和数据正确，视觉效果逐步改善。

## 预期测试

- 纯色材质、纹理材质、缺失纹理、UV 边界采样测试。
- 材质参数写入 G-Buffer 的数值测试。
- 法线贴图关闭/开启都不会破坏旧场景。
- 同一几何切换不同材质模型时输出有可解释差异。
- 现有 Blinn-Phong 兼容路径至少通过一个代表性渲染 smoke test。

## 预期产出

- 重构后的 material classes、texture sampler 或 texture resource 模块。
- 新版材质 schema 示例。
- 材质相关 G-Buffer 输出。
- 至少一个纯色材质场景和一个纹理材质场景。
- `.pipeline/architecture.md` 中记录材质系统边界和后续 PBR 扩展点。
