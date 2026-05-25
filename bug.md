# 待修复问题

当前会话恢复 ID：`019e4eb4-bf7a-7803-967f-d6c74249f05e`

bugfix 恢复 ID：`019e4f12-16cb-75e0-bc5d-ad11161dc362`

## 新增需求 / 缺陷

1. 搜索成功后，将键盘焦点移动到筛选结果
   - 状态：已完成（2026-05-25）
   - 类型：功能需求
   - 场景：用户在搜索框输入关键词并触发搜索后，搜索完成且有结果时，应能直接用键盘在筛选结果列表中滚动、选择或继续操作。
   - 需求：搜索成功后，将当前响应者 / 键盘焦点移动到筛选结果视图。
   - 验收：触发搜索并完成后，无需额外点击筛选结果区域，即可用方向键、PageUp/PageDown 等快捷键操作筛选结果。
   - 实现：完整搜索结束、非 follow 追加刷新且有匹配结果时，将焦点设置到筛选结果视图。

2. 在搜索框前增加“标记当前搜索结果”按钮
   - 状态：已完成（2026-05-25）
   - 类型：功能需求
   - 场景：用户已经执行一次搜索，希望将当前搜索命中的所有行快速加入标记。
   - 需求：在搜索输入框左侧增加一个按钮，用于标记当前搜索结果。
   - 验收：点击按钮后，当前搜索结果中的所有匹配行被标记；已有标记不重复计数；界面刷新后能在主视图、筛选视图和 overview 中看到标记状态。
   - 实现：搜索完成且有匹配时启用按钮；点击后标记当前匹配行，登记到“已标记”列表，并刷新主视图、筛选视图和 overview。

3. 版本显示仍为 `24.11.3.0`
   - 状态：已完成（2026-05-25）
   - 类型：缺陷
   - 问题：当前安装 / 运行的 app 显示版本仍是 `24.11.3.0`，无法区分本地修改构建。
   - 影响：验证本地安装包时，难以确认运行的是最新本地构建还是旧版本。
   - 建议：检查 CMake `BUILD_VERSION`、`klogg_version.h` 生成逻辑、macOS bundle 信息以及 About / 窗口标题的版本来源，确保本地构建能显示可识别的版本号或 commit 标识。
   - 实现：新增显示版本 `KLOGG_DISPLAY_VERSION`；本地构建显示为 `local-<git describe --dirty>-<构建时间>`，About、窗口标题和 CLI 使用显示版本，bundle 元数据继续保持 macOS 兼容的数字版本。

## 高 / 中优先级

1. `drawTextArea()` 在 paint 路径中调用 `updateScrollBars()`
   - 位置：`src/ui/src/abstractlogview.cpp`
   - 问题：绘制过程中修改滚动条 range/pageStep，可能触发滚动信号、重绘抖动或重入。
   - 建议：将固定前缀列导致的 margin / scrollbar 更新移出 paint 路径。

2. 固定前缀列可能导致可见列数为负
   - 位置：`src/ui/src/abstractlogview.cpp`
   - 问题：小窗口或前缀宽度较大时，`viewport width - leftMargin` 可能小于 0，后续横向滚动条和绘制逻辑可能异常。
   - 建议：给 `getNbVisibleCols()` 加下限，或限制固定前缀列实际宽度。

3. 主搜索高亮迁移覆盖用户偏好
   - 位置：`src/settings/src/configuration.cpp`
   - 问题：没有迁移标记的旧配置会被强制 `enableMainSearchHighlight_ = true`，即使用户之前明确关闭过。
   - 建议：只在旧 key 不存在时应用新默认值。

4. `changeFontSize()` 不再持久化字体大小
   - 位置：`src/ui/src/crawlerwidget.cpp`
   - 问题：旧行为会写入 `Configuration::mainFont`，新行为只改当前 view，重启后丢失。
   - 建议：明确语义；若是字体设置快捷键，应保存配置；若是临时 zoom，应调整命名、文案和重置逻辑。

## 中 / 低优先级

5. `getMatchingLinePortion()` 每次点击都重新编译正则
   - 位置：`src/logdata/src/logfiltereddata.cpp`
   - 问题：不在搜索热路径，但复杂正则和高频点击筛选结果时有额外开销。
   - 建议：用 `LogFilteredData` 成员缓存当前 `pattern + flags` 的 `QRegularExpression`。

6. `setFixedPrefixVisible()` 重复调用 `Configuration::get()`
   - 位置：`src/ui/src/abstractlogview.cpp`
   - 问题：低风险可读性问题。
   - 建议：合并为局部 `const auto& config = Configuration::get();`。

7. `ElasticHook::move()` 中冗余 `allowHook_` 判断
   - 位置：`src/ui/src/viewtools.cpp`
   - 问题：函数开头已经 early-return，后续条件里的 `allowHook_` 必然为 true。
   - 建议：简化条件以减少阅读噪音。

8. 筛选视图 follow 测试命名 / 意图不准确
   - 位置：`tests/ui/crawlerwidget_test.cpp`
   - 问题：`followSet(true)` 不再连接到 filtered view，测试实际验证的是“全局 follow 不影响筛选视图滚动”。
   - 建议：改测试名或补充注释说明真实意图。

## 低优先级 / 杂项

9. macOS 安装脚本 preset 可配置但产物路径硬编码
   - 位置：`scripts/build-install-macos-arm64.sh`
   - 问题：使用非默认 `KLOGG_CMAKE_PRESET` 时，可能从错误目录安装 app。
   - 建议：从 preset 的实际 binary dir 取产物，或移除可变 preset。

10. 版本 action 存在尾随空白
    - 位置：`.github/actions/klogg-version/action.yml`
    - 问题：`git diff --check 13265cd..HEAD` 报 trailing whitespace。
    - 建议：删除尾随空白。
