# M2: Scene Schema, CLI, and Output Contract

## 需求

建立新版 scene schema、命令行接口和统一输出目录结构。长期格式可以重新设计，不需要被旧 `configs/*.json` 绑定；旧配置只作为迁移参考。

目标输出结构：

```text
outputs/<scene-name>/
  color.png
  gbuffer/
  shadow/
```

实施计划：

1. 设计新版 JSON scene schema，至少包含 `scene.name`、`camera`、`objects` 或 `meshes`、`materials`、`lights`、`outputs`。
2. 明确 schema 的扩展边界：未来能加入多 camera、多 render pass、更多材质通道和更多 light type。
3. 实现命令行参数解析，至少支持 config path、output root、输出 buffer 选择、分辨率覆盖、debug/verbose 开关。
4. 引入 `OutputManager` 或等价模块，集中负责 scene 输出目录、子目录创建、文件命名和覆盖策略。
5. 重写或迁移 2-3 个代表性配置到新版 schema，优先选择简单场景、Cornell/Bunny/Dragon 中的至少一个。
6. 给 schema 解析添加清晰错误信息：缺字段、未知材质/光源类型、缺资源路径、非法输出选项。

约束：

- 本里程碑以结构和接口清晰为优先，不扩展材质和光源视觉能力。
- 旧配置可以不长期兼容，但需要保留迁移说明或新旧字段对照。
- CLI 和输出路径后续里程碑会依赖，避免把路径逻辑分散在渲染阶段里。

## 预期测试

- CLI smoke test：指定 config、output root、scene name 后，目录结构按约定创建。
- 新版 schema 能成功加载至少两个代表性场景。
- 缺失字段、未知 light/material、缺失 asset、非法 buffer 选择能返回明确错误。
- 分辨率覆盖参数能影响 camera/render target 尺寸。
- 输出目录命名不会让多光源、多 buffer 文件互相覆盖。

## 预期产出

- 新版 scene config 示例。
- CLI 参数解析代码和输出目录管理代码。
- README 中的新版运行方式说明。
- 旧配置迁移说明或字段映射笔记。
- `.pipeline/architecture.md` 中记录 scene/schema/output contract 变更。
