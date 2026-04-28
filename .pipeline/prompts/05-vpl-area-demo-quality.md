# M6: VPL Area Lighting, Demo Scenes, and Quality Pass

## 需求

完成 Area light 的 VPL 采样支持、重点展示多 VPL 和 Cube Shadow 场景，并做一次整体质量收敛。这个里程碑要让项目不只是有框架，还能通过代表性场景展示最终图片、G-Buffer、Depth、Shadow Map、VPL debug 输出。

实施计划：

1. 设计 Area light VPL 采样策略，支持配置采样数量、分布方式、随机/确定性 seed 和 debug 输出。
2. 将 VPL 作为可查询的 direct lighting samples 接入现有光照流程，确保 G-Buffer / visibility / shadow 输出能反映 VPL 信息。
3. 为 VPL 相关 shadow 或 visibility 输出设计稳定命名，重点展示多 VPL 与 Cube Shadow 的组合场景。
4. 准备代表性 demo configs：Cornell Box、Bunny/Dragon、多材质、多光源、Point cube shadow、Area VPL。
5. 做渲染质量 pass：shadow bias、gamma/tonemap、normal transform、depth range、错误信息、输出目录清理。
6. 做性能 smoke test：低/中分辨率 demo 不出现明显数量级退化；必要时加入采样数量上限或 debug 输出开关。
7. 在 README 或报告中记录使用命令、输出文件含义、示例图、已知限制和下一步方向。
8. 评估 Cascade Shadow Map 是否进入下一轮计划，并记录理由。

约束：

- VPL 采样需要较充分展示，不只是能编译的最小实现。
- 结构清晰仍优先于短期视觉 hack。
- 本里程碑可以做质量收敛，但不应引入大范围无测试重构。

## 预期测试

- VPL 数量、分布和 seed 能影响输出，并可通过 debug 数据验证。
- 代表性场景全部能用统一 CLI 渲染。
- 每个关键场景至少生成 color、gbuffer、shadow 中的必要输出。
- 低/中分辨率性能 smoke test 不出现明显数量级退化。
- README 中的命令能在当前仓库复现。

## 预期产出

- Area light VPL 实现和配置示例。
- 多 VPL / Cube Shadow 展示场景。
- 最终 demo configs、输出说明和 README 更新。
- 已知限制和下一步计划，包括 Cascade Shadow Map 决策。
- `.pipeline/architecture.md` 中记录 VPL、demo、质量收敛后的架构状态。
