# M7: Validation Dataset Manifest and Runner Foundation

## 需求

建立测试场景数据集的结构化清单、分层运行模型和批量渲染/验证基础入口。本里程碑不追求场景数量，而是先把 validation suite 的数据契约做稳，后续 fast、standard、heavy 场景都挂到同一套结构上。

实施计划：

1. 设计 `validation/scenes.yaml` manifest，记录 scene id、tier、tags、config、assets、expected outputs、runtime budget、source/license、是否默认运行。
2. 建立 `configs/validation/{fast,standard,heavy}/` 目录约定，保留已有 `configs/*.json` 和 smoke V2 configs 可运行。
3. 新增 validation runner 的第一版入口，可以枚举 suite、过滤 tier/tag、检查配置文件和资产存在性，并可调用现有 CLI 进行批量渲染。
4. 为后续外部下载工具预留 proxy 配置字段，明确支持 7890、7891、7892 端口，但不要在代码里硬编码必须使用代理。
5. 新增测试 target，覆盖 manifest 解析、tier/tag 过滤、缺失资产诊断和 runner dry-run。
6. 更新文档，说明 fast/standard/heavy 三层语义和默认回归策略。

约束：

- 不重排已完成的 00-05 prompts，本轮从 06 追加。
- daily/default 不应默认跑 heavy 场景。
- runner 第一版可以偏简单，但 manifest 字段必须能承载后续外部资源、baseline 和可视化需求。

## 预期测试

- Manifest parser 能验证必填字段、tier enum、expected outputs schema、缺失 config/asset 的错误信息。
- Batch runner 能枚举 fast/standard/heavy，默认不执行 heavy。
- 至少一个已有 smoke V2 config 能被 manifest 引用并通过 dry-run。
- `xmake run` 或等价命令中新增的 validation 测试通过。

## 预期产出

- `validation/scenes.yaml`
- `configs/validation/` 目录结构
- validation runner 基础脚本或 C++/Python helper
- `ValidationSceneTests` 或等价 xmake target
- `docs/validation-scenes.md`
- `.pipeline/architecture.md` 追加 M7 执行记录
