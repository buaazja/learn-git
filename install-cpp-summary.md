# C++ 运行问题解决总结

## 问题
在 VS Code 中运行 `c1.cpp` 时，出现 `g++` 或 `cl` 无法识别的错误，说明系统中没有可用的 C++ 编译器。

## 诊断步骤
1. 检查 `c1.cpp` 文件内容，确认代码正常。
2. 在终端执行 `g++ --version`，结果显示命令未找到。
3. 在终端执行 `cl`，结果显示命令未找到。
4. 尝试使用 `winget` 查找可用的 MinGW 包。
5. 尝试使用 `winget install` 安装 `MartinStorsjo.LLVM-MinGW.MSVCRT`。
6. 进一步尝试使用 PowerShell 下载并解压 MinGW，但遇到网络/证书问题。

## 尝试过的解决方案
- 通过 `winget` 安装 MinGW 编译器。
- 通过命令行直接下载 LLVM MinGW ZIP 包并解压。
- 尝试使用 `curl` 与 `bitsadmin` 进行下载。
- 更新 `.vscode/launch.json` 以增加调试配置。

## 结果
- 目前尚未成功自动完成编译器安装。
- 主要障碍是网络连接与下载校验问题，导致 `g++` 仍不可用。

## 最终建议
1. 手动下载安装 MinGW-w64：
   - 访问 https://www.mingw-w64.org/downloads/
   - 下载适合的安装程序并运行
   - 选择 `x86_64` 架构
2. 将 MinGW 安装目录中的 `bin` 路径添加到系统 `PATH`。
3. 重新启动 VS Code。
4. 在项目根目录打开 `c1.cpp`，按 F5 运行。

## 说明
如果需要，我可以继续帮您配置 `tasks.json` 或 `launch.json`，并验证 `g++` 是否已正确安装。