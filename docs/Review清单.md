# Review 清单

本文用于把后续整理工作切换到“先 review、再修改、最后验证”的节奏。Review 输出应优先列风险和证据，少写泛泛总结。

## Review 请求模板

单个提交：

```text
review HEAD~1..HEAD，不改代码，按严重程度列问题
```

一段版本差异：

```text
review v0.3.7..HEAD，重点看跨平台行为、构建链路和输入文本
```

指定模块：

```text
review core/render/text.cpp components/input.h，不改代码，重点看 emoji、光标、撤回和 IME
```

## 输出格式

Review 结果按这个顺序写：

- 严重问题：会导致崩溃、数据错误、平台不可用、release 失败的问题。
- 中等问题：行为回归、跨平台不一致、缺少关键验证的问题。
- 低风险建议：命名、文档、局部可读性或后续可做的清理。
- 验证情况：实际跑过的命令、未跑的命令和原因。

没有发现问题时也要明确写“未发现阻塞问题”，并说明剩余风险。

## 高风险区域

文本与输入：

- `core/render/text.cpp`：字体 fallback、HarfBuzz shaping、FreeType metrics、emoji 渲染、测量与绘制一致性。
- `components/input.h`：光标定位、选择区、undo/redo、剪贴板、IME composition、emoji 与多字节文本。
- 手动验证需覆盖英文、中文、emoji、混排、多行输入和连续撤回。

构建与依赖：

- `CMakeLists.txt`：`EUI_DEPS_MODE=auto|bundled|fetch`、平台分支、依赖下载地址和缓存目录。
- `3rd/`：只 review 集成方式、版本固定和构建影响，不 review 上游源码风格。
- `.github/workflows/release.yml`：tag 触发、三平台产物名、asset 上传范围。

平台与资源：

- `core/platform/platform.cpp`、`core/platform/ime_bridge.c`：窗口事件、输入法桥接、平台条件编译。
- `core/render/image.cpp`：PNG/SVG 加载、缓存、尺寸测量、失败路径。
- `assets/`、`docs/pic/`：确认是否真需要随源码提交，避免 release 源码包膨胀。

## 验证命令

常规代码改动：

```sh
cmake -S . -B /tmp/eui-bundled -DCMAKE_BUILD_TYPE=Release -DEUI_DEPS_MODE=bundled
cmake --build /tmp/eui-bundled --parallel
```

依赖链路改动：

```sh
cmake -S . -B /tmp/eui-fetch -DCMAKE_BUILD_TYPE=Release -DEUI_DEPS_MODE=fetch
cmake --build /tmp/eui-fetch --parallel
```

发布前：

```sh
git status --short
git archive --format=zip --output=/tmp/eui-source.zip HEAD
ls -lh /tmp/eui-source.zip
```

## 手动检查

- 打开 `gallery`，确认主页面、组件示例和图片示例正常。
- 输入框连续输入 emoji，确认光标不逐渐错位。
- 输入中文，确认 composition、提交文本和光标位置正确。
- 连续撤回和重做，确认文本、光标和选择区一起恢复。
- 检查 macOS、Windows、Linux 差异，尤其是字体、输入法和窗口事件。

## Release Review

发布前最后检查：

- 工作区干净，tag 指向预期提交。
- 三个平台 workflow 全部通过。
- Release assets 只包含运行包，不包含正文截图。
- GitHub 自动 Source code 包体积合理。
- Release 正文截图能在网页中显示。
- `docs/releases/` 保持本地草稿目录，不进入提交。
