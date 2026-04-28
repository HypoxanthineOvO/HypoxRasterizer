# M1: Build, Test, and Baseline Recovery

## 需求

恢复项目的可靠构建、测试和最小渲染验证入口，为后续重构建立安全网。当前 `Tests/` 里有手工测试源码，但 `xmake.lua` 中测试 target 被注释，不能作为稳定回归依据；本里程碑需要把基础验证变成可重复执行的工程能力。

实施计划：

1. 梳理 `xmake.lua`，保留现有 `Utils`、`Rasterizer`、`HypoxRasterizer` target，并新增或恢复基础测试 target。
2. 将 `Tests/CameraTest.cpp`、`Tests/ImageTest.cpp`、`Tests/ObjectTest.cpp`、`Tests/TriangleTest.cpp` 中适合自动化的部分改成带断言和失败码的测试。
3. 增加一个轻量测试工具层，优先使用标准库断言或简单 test macro，不引入重型测试框架，除非确实更清晰。
4. 覆盖 geometry barycentric、camera view/projection、image write、config parse 的最小回归断言。
5. 增加一个低分辨率 smoke render 场景，确认主程序能完整跑完并生成当前阶段已有输出。
6. 给缺失 config 文件或非法 JSON 增加至少一个失败路径验证；如果当前代码无法优雅失败，应记录并做最小修复。

约束：

- 不在本里程碑重构渲染架构、材质系统或光源系统。
- 不改变现有 scene assets 的目录含义。
- 后续所有里程碑都应能复用这里建立的构建和测试命令。

## 预期测试

- `xmake` 构建主目标成功。
- 新增的基础测试 target 全部通过。
- smoke render 能生成当前等价的 color/depth/normal 输出或明确记录当前输出能力。
- 缺失 config 文件或非法 JSON 至少有一个可检测失败用例。
- 现有代表性配置至少有一个仍能用于最小渲染验证。

## 预期产出

- 更新后的 `xmake.lua` 测试 target。
- 自动化基础测试源码或测试入口。
- 低分辨率 smoke render 配置。
- README 或注释中记录构建、测试、smoke render 命令。
- 如发现架构风险，记录到 `.pipeline/architecture.md` 的后续评审备注中。
