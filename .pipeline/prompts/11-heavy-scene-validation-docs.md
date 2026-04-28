# M12: Heavy Scene Validation and Documentation Pass

## 需求

把 heavy 场景、外部经典场景和最终验证文档收束成可执行但不拖慢日常回归的验证层。Dragon、Bistro、Sponza、San Miguel 等场景需要有明确入口、预算、缺失资产处理和后续运行路径；至少一个内部 heavy/medium-heavy 场景应能在降低分辨率后跑通。

实施计划：

1. 为 Dragon、Bistro/Sponza/San Miguel 等 heavy 候选建立 optional configs 或 source hooks。
2. 设定 heavy runtime/resolution budgets，记录三档命令：fast default、standard validation、heavy benchmark。
3. 对至少一个内部 heavy/medium-heavy 场景进行低分辨率实际渲染验证，并记录性能统计。
4. external heavy assets 缺失时，runner 应报告 skip 和下载指引，不应破坏默认验证。
5. 汇总场景 suite 文档：每个场景验证点、命令、预期输出、资源来源、license、运行成本、已知限制。
6. 确认哪些场景进入默认回归、哪些进入 release/benchmark 手动运行。
7. 更新 pipeline architecture 和最终报告，明确下一步：完整 direct SceneConfig 渲染、外部格式转换、BRDF/shadow 质量、Cascade Shadow Map。

约束：

- heavy 不进入 daily default，但不能没有运行路径。
- license 不清晰的外部资源不能标记为 supported。
- 文档命令必须与实际 runner 名称一致。

## 预期测试

- Heavy suite 能 enumerate，并能 cleanly skip 缺失 external assets。
- 至少一个内部 heavy/medium-heavy 场景低分辨率跑通并产出 color/gbuffer/shadow。
- perf metadata 写入 `validation/perf/` 或等价位置。
- docs 中的核心命令能被 runner dry-run 验证。

## 预期产出

- `configs/validation/heavy/*.v2.json` 或 source hooks
- `validation/perf/*.yaml`
- finalized `docs/validation-scenes.md`
- README validation suite section
- `.pipeline/reports/11-heavy-scene-validation-docs.md`
- `.pipeline/architecture.md` 追加 M12 执行记录
