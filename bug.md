# 待修复问题

审查范围：`25c7de6..HEAD`

## Bug / 性能问题

1. 自动标记行归属查询是 `matches × searches` 扫描
   - 优先级：P2 / 中
   - 类型：数据量相关性能风险
   - 位置：`src/ui/src/crawlerwidget.cpp:1183`、`src/ui/src/crawlerwidget.cpp:1286`
   - 场景：用户保留多个自动标记搜索，并在大量匹配行上重复标记或删除标记。
   - 问题：`markCurrentMatchesAsAutoMarked()` 对每个匹配行调用 `isAutoMarkedLine()`，后者遍历当前 tab 的所有搜索词再查每个 `QSet`。搜索词和匹配数都大时复杂度约为 `matches × searches`。
   - 影响：大结果集下标记、移除自动标记搜索和刷新 UI 可能出现明显卡顿。
   - 建议：维护按 line 汇总的 owner 计数或 owner 集合，热路径按行号 O(1) 判断是否仍由其他自动标记搜索拥有。

## 验证缺口

1. 缺少固定前缀继承在长 continuation block 中连续滚动的性能/行为测试。
2. 缺少多个自动标记搜索共享同一行时，删除单个搜索与删除全部搜索的回归测试。
