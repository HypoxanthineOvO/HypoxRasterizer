# M8: Fast Corner Cases and Standard Internal Scenes

## 需求

手写高质量 fast corner cases，并把已有内部资产整理成 standard validation scenes。目标是让默认验证不只跑简单三角形，也能覆盖纹理、经典场景、Dragon/Bunny 级 mesh、多光源和 shadow/VPL 输出。

实施计划：

1. 设计 fast corner cases：空场景、屏幕外几何、退化三角形、极端 depth、无光源、单光源、多光源、cube shadow 六方向覆盖、缺失资源错误场景。
2. 将 fast 场景写成 V2 validation configs，放入 `configs/validation/fast/`，并加入 manifest。
3. 整理 standard 内部场景：CornellBox、Bunny、Dragon quick、Money/Nylon 纹理场景、Objects 基础几何组合。
4. 为 Dragon quick 设计低分辨率/低预算配置，要求能较快运行，不能只作为不可执行 heavy 场景。
5. 每个场景写清楚验证点：材质/纹理、G-Buffer、Depth、Directional/Point/Area、Cube Shadow、VPL debug、错误处理。
6. runner 支持按 suite 跑 fast 和 selected standard，并输出每个场景的成功/失败摘要。

约束：

- legacy configs 不要求全量迁移，只挑代表性资产做高质量 V2 configs。
- 小图形场景用于 corner case，不替代标准纹理/复杂 mesh 验证。
- standard 层必须至少包含一个纹理场景和一个 Dragon/Bunny 级别 mesh 场景。

## 预期测试

- fast suite 能端到端运行，并验证必要的 color、gbuffer、shadow、depth 输出。
- selected standard suite 至少跑通纹理场景、Cornell/Bunny/Dragon quick 中的代表项。
- 缺失资源/非法配置场景产生明确诊断，不应被误判为成功。
- 输出目录遵守 `outputs/<scene-name>/{color.png,gbuffer/,shadow/}`。

## 预期产出

- `configs/validation/fast/*.v2.json`
- `configs/validation/standard/*.v2.json`
- 更新后的 `validation/scenes.yaml`
- scene expectation metadata
- runner 文档和测试更新
- `.pipeline/architecture.md` 追加 M8 执行记录
