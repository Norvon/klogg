# Repository Guidelines

## 项目结构与模块组织

Klogg 是基于 CMake 的 C++17 / Qt 桌面应用。主源码位于 `src/`，按功能拆分为 `app`、`ui`、`logdata`、`regex`、`settings`、`utils` 等模块；每个模块通常包含 `include/`、`src/` 和 `CMakeLists.txt`。测试位于 `tests/`，其中 `tests/unit/` 放单元测试，`tests/ui/` 放 Qt UI/集成测试，`tests/helpers/` 放测试辅助代码。图标、字体和 Qt 资源在 `Resources/` 与 `src/app/` 下；打包脚本位于 `packaging/`，站点文档位于 `website/`。

## 构建、测试与开发命令

常规本地构建：

```bash
mkdir -p build_root
cd build_root
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
cmake --build .
```

macOS 使用 Ninja 和 Homebrew Qt 时，可参考：

```bash
cmake -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQt5_DIR=<qt-cmake-dir> ..
cmake --build .
```

构建产物输出到 `build_root/output`。运行测试：

```bash
cd build_root
ctest --build-config RelWithDebInfo --verbose
```

如需禁用测试配置，使用 `-DKLOGG_BUILD_TESTS=OFF`；如需禁用 Hyperscan/启用其他正则后端，参考 `BUILD.md` 中的 CMake 选项。

## 编码风格与命名约定

项目使用 C++17 和 Qt。遵循现有模块风格，头文件放 `include/`，实现放 `src/`，新增源文件需接入对应 `CMakeLists.txt`。仓库提供 `.clang-format`、`.clang-tidy` 和 `.cmake-format`；提交前优先格式化本次修改的文件，不做无关重排。类名使用既有 C++/Qt 风格，测试文件沿用 `*_test.cpp` 命名。

## 测试指南

测试使用 Catch2 与 QtTest，通过 CTest 注册为 `klogg_tests` 和 `klogg_itests`，并以 `-platform offscreen` 运行。新增核心逻辑优先补充 `tests/unit/`；涉及 UI、文件加载、过滤或主窗口行为时补充 `tests/ui/`。测试数据放入 `test_data/` 或测试目录附近，避免依赖本机绝对路径。

## 提交与 Pull Request 指南

提交信息遵循 `prefix: message`，常见前缀包括 `feat`、`fix`、`docs`、`refactor`、`perf`、`test`、`build`、`ci`、`chore`、`revert`、`tr`。示例：`fix: handle empty search pattern`。PR 应保持小而聚焦，说明变更目的、影响平台、验证命令和相关 issue；UI 变更请附截图或录屏。安全问题不要公开发 issue，按 `SECURITY.md` 联系维护者。
