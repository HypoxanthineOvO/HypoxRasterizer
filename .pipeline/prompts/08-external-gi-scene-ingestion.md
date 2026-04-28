# M9: External GI Scene Source Ingestion

## 需求

实现可审计的外部经典/GI 场景下载、许可记录和可转换子集。候选来源包括 Bitterli Rendering Resources、PBRT v4 scenes、Morgan McGuire Computer Graphics Archive / NVIDIA ORCA。第一批优先选中小型、格式可处理、许可清晰的资源；Bistro/San Miguel 等先做 optional/heavy hook，但后续仍需能跑。

实施计划：

1. 建立 `validation/external_sources.yaml`，记录 source id、URL、license、format、hash、tier、下载目标、转换方式和启用状态。
2. 建立统一 uv Python 工具项目 `tools/python/pyproject.toml`，模块包括 assets、validation、viz。
3. 实现 `tools/python/.../fetch_scenes.py` 或等价 CLI，支持 dry-run、下载、hash 校验、跳过已存在、代理环境变量，以及 7890-7892 端口配置。
4. 对外部资源下载路径使用 `assets/external/` 或可配置 cache，避免将大型资源直接纳入 git。
5. 优先接入 1-2 个小/中型可转换资源；对 PBRT/ORCA heavy source 先完成 metadata、optional fetch 和 conversion hook。
6. 明确 license/source 文档，未确认 license 的资源不能标记为默认 runnable。

约束：

- 需要联网下载时使用可配置代理；不要把本机代理写死为唯一可用路径。
- 外部资源必须有来源、许可和 hash 策略。
- 如果格式转换超出当前能力，先降级为 metadata + hook，不阻塞整体 suite。

## 预期测试

- fetcher dry-run 打印 planned downloads、license、target path、hash policy。
- fetcher 支持 proxy config，不使用代理时也能清晰报错或跳过。
- source manifest 校验缺失 license/hash/URL 时失败。
- optional/heavy 外部资产缺失时，runner 能 cleanly skip 而不是失败整个 default suite。

## 预期产出

- `tools/python/pyproject.toml`
- `tools/python/hypo_assets/` 或等价模块
- `validation/external_sources.yaml`
- `assets/external/.gitignore`
- 外部场景下载/转换文档
- `.pipeline/architecture.md` 追加 M9 执行记录
