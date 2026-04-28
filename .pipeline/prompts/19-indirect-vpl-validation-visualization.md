# M20: IndirectVPL Validation and Visualization

## 需求

为 IndirectVPL 建立可解释的验证场景、指标和 2D/3D 可视化产物，形成一份可比较的实验汇报。

实施计划：

1. 场景覆盖：
   - Cornell-style：直接光、direct area VPL、IndirectVPL、shadow/visibility on/off。
   - Textured scene：检查间接光是否带材质颜色。
   - Dragon quick：检查中等复杂 mesh 的 sample/gather 成本。
2. 输出对比：
   - color contact sheet。
   - direct-only vs IndirectVPL diff。
   - visibility on/off diff。
   - sample count sweep，例如 64/256/1024。
3. Python 可视化：
   - 2D overlay：IndirectVPL 采样点投影、按 flux 着色。
   - 3D view：first-bounce 采样点、normal glyph、source light id、shadow/frustum。
4. 指标：
   - sample accepted/rejected count。
   - mean/max indirect radiance。
   - render time。
   - diff mean/max。
5. 报告明确限制：one-bounce、能量估计粗略、visibility 成本、RSM/多反弹后续路线。

约束：

- 不把大型生成图纳入 git。
- heavy scene 默认可手动或 benchmark 层运行。
- 结果必须和 M13-M16 3D 可视化工具互相复用，不再写一套独立脚本。

## 预期测试

- Validation runner 能选择 IndirectVPL 场景并生成对比输出。
- Python tools 能读取 indirect sample `.f32` 并生成 2D/3D 标注。
- Baseline stats 能记录 IndirectVPL 输出并做容差比较。
- 最终报告列出每张图对应的场景内容、开关状态和指标。

## 预期产出

- IndirectVPL validation configs。
- 2D/3D 可视化集成。
- `reports/indirect-vpl-validation-*.md`
- `.pipeline/reports/19-indirect-vpl-validation-visualization.md`
