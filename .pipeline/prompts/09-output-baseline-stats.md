# M10: Output Statistics, Hashes, and Tolerance Baselines

## 需求

为 validation scene suite 建立可复现输出基线，覆盖文件存在、`.f32` header/schema、数值统计、PNG/hash 和容差比较。目标是让场景验证不依赖肉眼检查，并能在后续材质、光照、shadow 改动时发现明显回归。

实施计划：

1. 实现 Python `.f32` reader，解析 `HYPORASTER_F32_V1` header、name、width、height、channels 和 raw float payload。
2. 实现 PNG 和 float buffer stats collector，统计 min/max/mean/std/nonzero ratio、finite ratio、hash、shape、channels。
3. 设计 baseline 文件格式，放在 `validation/baselines/`，关联 scene id、renderer command、output files、stats 和 tolerance。
4. 实现 comparator，支持 strict hash 模式和 tolerance stats 模式；默认使用容差统计，避免平台浮点细节导致误报。
5. 将 fast suite baseline 纳入默认回归；standard baseline 用显式命令运行。
6. 输出差异报告，指出缺失文件、header mismatch、shape mismatch、hash mismatch、统计漂移。

约束：

- 不要把大 PNG 或大 `.f32` golden 文件直接纳入 git，优先存小型 stats baseline。
- baseline 更新必须是显式命令，不应在测试失败时自动覆盖。
- float NaN/inf、全零、常量 depth 等边界需要明确处理。

## 预期测试

- Stats collector 能解析当前 `.f32` 输出，并检测 malformed header。
- Baseline comparator 能捕捉缺失输出、shape mismatch、gross numeric drift 和 strict hash mismatch。
- Fast suite baseline generation 和 comparison 可重复运行。
- 至少一个 shadow map、一个 G-Buffer channel、一个 VPL debug buffer 进入 baseline。

## 预期产出

- `tools/python/hypo_validation/collect_stats.py`
- `tools/python/hypo_validation/compare_baseline.py`
- `validation/baselines/*.yaml` 或 `.json`
- baseline generation/update/compare 文档
- validation baseline 测试 target
- `.pipeline/architecture.md` 追加 M10 执行记录
