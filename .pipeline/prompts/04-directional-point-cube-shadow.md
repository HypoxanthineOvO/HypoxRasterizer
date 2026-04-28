# M5: Directional/Point Lights and Cube Shadow Maps

## 需求

建立主流直接光和 shadow map 管线，优先支持 Directional light、Point light 和 Point light Cube Shadow Map。当前代码已有点光源六方向 shadow map 的雏形，但需要统一资源结构、输出命名、visibility 查询和 debug 逻辑。

实施计划：

1. 重构 `Light` 接口，明确 light type、sampling、shadow resource、visibility query、debug output 的职责边界。
2. 实现 Directional light，并支持对应 shadow map；先做稳定单级 shadow map，Cascade Shadow Map 不进本里程碑。
3. 实现 Point light cube shadow map，整理当前六方向 shadow map 逻辑，明确 face orientation、projection、depth range 和输出命名。
4. 将 shadow map 输出统一到 `outputs/<scene-name>/shadow/`，多光源、多 face 文件不互相覆盖。
5. 将 shadow visibility 写入 G-Buffer 或 debug buffer。
6. 实现基础 shadow bias，并为 PCF 留出清晰扩展点；如果实现 PCF，需要有开关和测试。
7. 减少 `ShadowMap` 和主 rasterizer 中重复或分叉的 rasterization 逻辑，至少明确共享边界。

约束：

- Directional 和 Point 是必做；Spot 可以后续再进。
- Cube Shadow Map 是重点能力，不能只保留无组织的六张 PNG。
- Cascade Shadow Map 在 M6 后再评估，不要提前牵扯本里程碑范围。

## 预期测试

- Directional light 阴影方向、深度范围、可见性测试。
- Point light cube shadow 六个 face 覆盖正负轴方向测试。
- 多光源 shadow 输出命名稳定且不覆盖。
- shadow visibility 能写入 G-Buffer 或 debug 输出。
- 至少一个场景展示有遮挡和无遮挡片元的可见性差异。

## 预期产出

- 重构后的 light/shadow map 接口。
- Directional light shadow 示例。
- Point cube shadow 示例。
- `outputs/<scene-name>/shadow/` 下结构化 shadow debug 输出。
- `.pipeline/architecture.md` 中记录 light/shadow resource 设计和 Cascade 后续决策点。
