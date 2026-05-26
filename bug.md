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

## 审查新增高优先级缺陷（2026-05-25）

1. Qt5 vendored KArchive 构建会被 Qt6 专用补丁破坏
   - 状态：已完成（2026-05-25）
   - 优先级：P1 / 高，需优先阻断合入
   - 类型：兼容性构建缺陷
   - 位置：`3rdparty/CMakeLists.txt`
   - 场景：使用受支持的 Qt5 构建，并通过 CPM 拉取 vendored `klogg_karchive`。
   - 问题：当前 `PATCH_COMMAND` 无条件应用 `cmake/patch_karchive_qt6.cmake`，会把 KArchive 源码改成调用 `QFlags::toInt()`；该 API 只存在于 Qt6，Qt5 依赖隐式整数转换，因此 Qt5 构建会失败。
   - 影响：破坏受支持的 Qt5 构建路径，属于平台 / 版本兼容性回归。
   - 建议：仅在 `QT_VERSION_MAJOR == 6` 时应用该补丁，或改为 Qt5 / Qt6 都兼容的转换写法。
   - 实现：`PATCH_COMMAND` 改为仅在 Qt6 构建时传入 CPM；Qt5 vendored KArchive 路径不再应用 Qt6 专用补丁。
   - 验收：Qt5 使用 vendored KArchive 时不再应用 Qt6 专用 `toInt()` 补丁并能编译；Qt6 vendored KArchive 构建仍保持当前修复效果。

2. 自动标记搜索状态在保留结果 tab 之间串状态
   - 状态：已完成（2026-05-25）
   - 优先级：P2 / 高，排在 Qt5 构建阻断之后
   - 类型：状态管理 / 行为回归
   - 位置：`src/ui/include/crawlerwidget.h`
   - 场景：用户开启“保留结果”并创建多个筛选结果 tab，然后在不同 tab 中使用“标记当前搜索结果”并删除某个已标记搜索。
   - 问题：`autoMarkedSearches_` 和 `autoMarkedLinesBySearch_` 是 `CrawlerWidget` 级共享状态，不随当前 `LogFilteredData` / 结果 tab 独立保存。切换到另一个 tab 后删除已标记搜索，会对当前数据集调用 `deleteMark`，但 UI 删除的是全局唯一条目，原 tab 中的自动标记可能残留且失去 UI 管理入口。
   - 影响：多结果 tab 下标记状态和 UI 列表不一致，用户无法可靠撤销原 tab 中的自动标记。
   - 建议：将自动标记搜索列表和行集合按 filtered view / `LogFilteredData` 存储；或在 tab 切换时明确重新绑定、刷新或清理当前 tab 对应状态。
   - 实现：自动标记搜索状态改为按 `FilteredView` 保存，tab 切换时刷新当前 tab 的 UI；新增 kept result tab 回归测试。
   - 验收：多个保留结果 tab 中分别自动标记后，删除某个 tab 的已标记搜索只影响该 tab 对应数据；切换 tab 后 UI 列表与当前数据集的实际标记状态一致。

## 高 / 中优先级

1. `drawTextArea()` 在 paint 路径中调用 `updateScrollBars()`
   - 状态：已完成（2026-05-26）
   - 位置：`src/ui/src/abstractlogview.cpp`
   - 问题：绘制过程中修改滚动条 range/pageStep，可能触发滚动信号、重绘抖动或重入。
   - 建议：将固定前缀列导致的 margin / scrollbar 更新移出 paint 路径。
   - 实现：滚动过程中检测到 margin 变化时只排队刷新滚动条，避免在当前滚动 / 绘制相关调用链里同步修改 scrollbar。

2. 固定前缀列可能导致可见列数为负
   - 状态：已完成（2026-05-26）
   - 位置：`src/ui/src/abstractlogview.cpp`
   - 问题：小窗口或前缀宽度较大时，`viewport width - leftMargin` 可能小于 0，后续横向滚动条和绘制逻辑可能异常。
   - 建议：给 `getNbVisibleCols()` 加下限，或限制固定前缀列实际宽度。
   - 实现：可见文本宽度先夹到非负值；固定前缀列数按当前 viewport 可用宽度动态收窄，并至少保留一个正文列。

3. 主搜索高亮迁移覆盖用户偏好
   - 状态：已完成（2026-05-26）
   - 位置：`src/settings/src/configuration.cpp`
   - 问题：没有迁移标记的旧配置会被强制 `enableMainSearchHighlight_ = true`，即使用户之前明确关闭过。
   - 建议：只在旧 key 不存在时应用新默认值。
   - 实现：当前实现已先检查 `regexpType.mainHighlight` 是否存在；只有旧配置缺少该 key 时才写入新默认值，不覆盖用户已有设置。

4. `changeFontSize()` 不再持久化字体大小
   - 状态：已完成（2026-05-26）
   - 位置：`src/ui/src/crawlerwidget.cpp`
   - 问题：旧行为会写入 `Configuration::mainFont`，新行为只改当前 view，重启后丢失。
   - 建议：明确语义；若是字体设置快捷键，应保存配置；若是临时 zoom，应调整命名、文案和重置逻辑。
   - 实现：当前实现已在字号变更后调用 `Configuration::setMainFont()` 并 `save()`，随后用配置字体刷新所有视图。

5. `getMatchingLinePortion()` 每次跳转都重新取行并执行正则匹配
   - 状态：已完成（2026-05-25）
   - 位置：`src/logdata/src/logfiltereddata.cpp`
   - 优先级：高 / 中，主线程交互路径性能风险
   - 问题：当前已经缓存 `QRegularExpression`，不再是“每次重新编译正则”；但每次点击筛选结果跳转时仍会重新读取原始行并执行一次 `regexp.match()`。复杂正则、大行文本或高频点击时，可能在主线程产生可感知延迟。
   - 建议：确认这是有意取舍还是遗漏；若需要优化，可按 `search key + lineNumber` 缓存匹配区间，或在构建筛选结果时保存首个匹配位置。
   - 实现：在当前正则缓存内增加按源日志行号保存的匹配区间缓存，搜索清空 / 重建时随正则缓存一起失效。

6. `findPreviousFixedPrefix()` 在绘制路径中可能反复回溯读取大量行
   - 状态：已完成（2026-05-25）
   - 位置：`src/ui/src/abstractlogview.cpp`
   - 优先级：高 / 中，渲染路径性能风险
   - 问题：`drawTextArea()` 每次绘制首行且首行没有固定前缀时，会向上最多回溯 `FixedPrefixLookbackLines`（当前 5000 行）并调用 `logData_->getLines()`。结果没有缓存，滚动或重绘时可能持续触发批量读取和正则匹配。
   - 建议：缓存上一次 `firstLine_` 对应的回溯结果，并在固定前缀配置、数据源变化或首行跨越缓存范围时失效。
   - 实现：为固定前缀继承增加按数据源、配置和 `firstLine_` 绑定的回溯结果缓存；数据更新或固定前缀配置变更时失效。

## 中 / 低优先级

7. `markCurrentSearchResults()` 与 `updateFilteredView()` 中自动标记逻辑重复
   - 状态：已完成（2026-05-25）
   - 位置：`src/ui/src/crawlerwidget.cpp`
   - 优先级：中，维护性风险
   - 问题：两处都有“遍历匹配行、添加 mark、更新 `autoMarkedLinesBySearch_`、刷新 UI”的近似逻辑。后续修复自动标记的归属、去重或刷新规则时，容易只改一处造成行为分叉。
   - 建议：提取私有方法承载自动标记当前匹配结果的核心逻辑，调用方只负责不同入口的状态准备和提示文案。
   - 实现：提取 `markCurrentMatchesAsAutoMarked()` 和 `refreshViewsAfterMarksChanged()`，手动标记和搜索完成后的自动标记共用同一逻辑。

8. `isAutoMarkedLine()` 对所有自动标记搜索做 O(n*m) 扫描
   - 位置：`src/ui/src/crawlerwidget.cpp`
   - 优先级：中 / 低，数据量相关性能风险
   - 问题：每个 tab 的自动标记状态仍使用 `QHash<QString, QSet<uint64_t>>` 保存，`isAutoMarkedLine()` 需要遍历当前 tab 的所有搜索词再查每个集合。自动标记大量行时，`iterateOverMatches()` 对每一行调用该函数，复杂度约为 `matches × searches`。
   - 建议：搜索词数量很少时影响有限；若支持大量自动标记搜索，可维护一份按 line 汇总的 owner 计数 / 集合，避免热路径扫描全部搜索词。

9. `setFixedPrefixVisible()` 重复调用 `Configuration::get()`
   - 位置：`src/ui/src/abstractlogview.cpp`
   - 问题：低风险可读性问题。
   - 建议：合并为局部 `const auto& config = Configuration::get();`。

10. `ElasticHook::allowHook(false)` 的调用路径还需要确认
   - 位置：`src/ui/src/viewtools.cpp`
   - 问题：`move()` 在 `allowHook_ == false` 时 early-return 是合理防御，但如果此时仍有调用方持续调用 `move()`，说明调用路径可能仍未完全解除状态泄漏。
   - 建议：确认 `allowHook(false)` 期间的调用来源；若调用是预期的，保留防御并补充注释，否则修正调用方状态流。

11. 筛选视图 follow 测试命名 / 意图不准确
   - 位置：`tests/ui/crawlerwidget_test.cpp`
   - 问题：`followSet(true)` 不再连接到 filtered view，测试实际验证的是“全局 follow 不影响筛选视图滚动”。
   - 建议：改测试名或补充注释说明真实意图。

## 低优先级 / 杂项

12. `ScreenBoundComboBox` 局部类缺少作用域说明
    - 位置：`src/ui/src/crawlerwidget.cpp`
    - 问题：类定义在 `.cpp` 文件中是可接受的局部实现细节，但用途和不放入头文件的原因不明显。
    - 建议：加一行短注释说明该类只服务当前搜索框弹层约束，暂不作为通用控件暴露。

13. `SavedSearches::frequentSearches()` 对缺失 usageCount 默认返回 1 的语义较微妙
    - 位置：`src/ui/src/savedsearches.cpp`
    - 问题：历史记录中没有 `usageCount` 的条目默认按 1 参与常用排序；`resetUsageCount()` 会写入 0，使条目从常用列表消失但仍留在历史记录中。这是合理行为，但和 `usageCount()` 返回 0 表示“已重置 / 不常用”的语义需要读代码才能理解。
    - 建议：补一行注释说明默认 1 是为了兼容旧历史记录，显式 0 表示用户重置后隐藏于常用列表。

14. `linetypes.h` 中 UDL 空格变更属于纯格式改动
    - 位置：`src/logdata/include/linetypes.h`
    - 问题：`operator"" _lnum` 改成 `operator""_lnum` 符合更新的写法，但与本次功能无直接关系，会增加 blame 噪音。
    - 建议：若还未合入，拆到独立格式提交；若已合入，后续避免把纯格式变化混进功能修复。

15. `saveLinesToFile()` 的 `open()` 返回值检查应独立追踪
    - 位置：`src/ui/src/abstractlogview.cpp`
    - 问题：这是正确的 bug 修复：`QSaveFile::open()` 失败时不应继续写入流程。
    - 建议：作为独立 bugfix 提交或在 PR 说明中单独列出，便于回溯错误处理修复。

16. `installLanguage()` 中 Qt 翻译缺失后继续执行的语义需要确认
    - 位置：`src/ui/src/mainwindow.cpp`
    - 问题：当前逻辑在 `qt_<lang>.qm` 缺失时只记录 warning，并继续加载应用自身翻译；旧逻辑若把 Qt 翻译缺失视为失败，行为会发生变化。
    - 建议：确认这是预期兼容行为；如果是，补测试或注释说明“应用翻译存在时不强依赖 Qt 翻译”。

17. macOS 安装脚本 preset 可配置但产物路径硬编码
   - 位置：`scripts/build-install-macos-arm64.sh`
   - 问题：使用非默认 `KLOGG_CMAKE_PRESET` 时，可能从错误目录安装 app。
   - 建议：从 preset 的实际 binary dir 取产物，或移除可变 preset。

18. 版本 action 存在尾随空白
    - 位置：`.github/actions/klogg-version/action.yml`
    - 问题：`git diff --check 13265cd..HEAD` 报 trailing whitespace。
    - 建议：删除尾随空白。

## 测试覆盖缺口

1. 多个自动标记搜索共享同一行时，缺少 `removeAutoMarkedSearch()` / `hasOtherAutoMarkOwner()` 的回归测试。
   - 风险：删除一个搜索词时，可能误删仍由其他自动标记搜索拥有的 mark，或在多 tab 状态下留下孤儿 mark。

2. 固定前缀继承的回溯行为缺少测试。
   - 风险：后续为 `findPreviousFixedPrefix()` 增加缓存时，容易破坏“首行无前缀时继承上方最近前缀”的行为。

3. `getMatchingLinePortion()` 对 boolean / exclude 模式缺少边界测试。
   - 风险：这些模式当前返回空匹配区间；后续优化匹配区间缓存时，需要确保仍不会为 boolean / exclude 搜索生成错误跳转范围。
