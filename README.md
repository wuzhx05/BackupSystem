# Backup System

## 功能

该备份系统提供以下功能：

1. **文件备份**：将源文件备份到指定的目录。
   - `PATH_BACKUP_COPIES`指定了备份副本的存放目录，一般为`./backup_copies`。
   - `PATH_BACKUP_DATA`指定了备份数据（源文件路径、大小、MD5等信息）的存放目录，格式为`./backup_v{VERSION}`，其中`{VERSION}`是备份系统的版本号。
   - **错误检查**：在备份程序退出前检查源文件和备份文件的状态，包括文件是否存在、文件大小是否一致、文件大小是否变化以及修改时间是否一致。
3. **日志记录**：记录备份过程中的操作和重要信息。
4. **字符串编码**：根据用户输入自适应调整输出编码。默认控制台编码为**`UTF-8`**，Windows下终端编码可能为`GBK`，但用户输入均为`ASCII`，则无法调整，程序输出仍为`UTF-8`。

## 备份检查（`check`）错误代码

错误代码为二进制含义，从最高有效位（MSB）到最低有效位（LSB）的含义如下：

|  `4` | `3`  |  `2` | `1`  | `0`  |
| ---- | ---- | ---- | ---- | ---- |
| 源文件不存在 | 备份文件不存在 | 源、备份文件大小不一致 | 源文件大小变化 | 修改时间不一致 |

## 项目框架

### 目录结构

- `backup/`：包含备份相关的源代码和头文件。
- `restore/`：包含恢复相关的源代码和头文件。
- `share/`：包含共享的源代码和头文件。
- `lib/` 或 `lib_mingw/`：包含项目依赖的库文件，主要是CED库。
- `CMakeLists.txt`：CMake构建脚本。

### 源代码文件

- `backup/main.cpp`：备份程序的入口点。
- `backup/src/head.cpp`：`backup/main.cpp`的模块化实现。
- `restore/main.cpp`：恢复程序的入口点。

share:

- `share/src/config.cpp`：配置相关的实现代码，统领全局。
- `share/src/env.cpp`：运行环境：程序调用时间；~~UUID（暂未启用）~~。
- `share/src/str_encode.cpp`：字符串编码检测与转换。
- `share/src/print.cpp`：ANSI颜色序列转义，**同步打印**，日志输出，打印进度条。**输出务必使用此文件定义的函数**。
- `share/src/file_info.cpp`：文件信息类。
- `share/src/file_info_md5.cpp`：文件信息类补充：MD5计算的实现代码。
- `share/src/ThreadPool.cpp`：线程池、异步拷贝文件。

## 依赖项目

该备份系统依赖以下项目：

- [OpenSSL](https://www.openssl.org/)：用于计算MD5。
- [Boost](https://www.boost.org/)：
  - `program_options`：用于解析命令行参数。
  - `locale`：用于处理字符串编码转换。
- [Compact Encoding Detection](https://github.com/google/compact_enc_det)：用于识别字符串编码。

## 待办事项

- [x] 编写注释
- [ ] 编写clean
- [ ] 更优的字符串相似度匹配
- [ ] 统一代码风格：命名方式
- [ ] 更完善的线程池
- [ ] copyright 信息？
- [ ] 发布到github
- [x] 定义版本号
- [x] 拷贝文件前检查时间戳
- [x] 日志文件
- [x] 字符串编码
- [x] MD5`BUFFERSIZE`放到`config.hpp`
- [x] `log`单独路径
- [x] 根据历史信息得出MD5(Cache)，跳过计算；编写测试代码。
- [x] `file_info`使用`fs::path`。
- [x] 优化编码检测
- [x] `fs::canonical` for backup
- [x] 编写restore

### 长远

- [ ] 对于文件大小部分，更丝滑的进度更新
- [ ] 服务器端口传输
- [ ] 缓存文件完整性和被篡改检查
- [ ] UUID生成
- [ ] 文件属性：隐藏等

## 项目部署

克隆仓库并保证安装必要[依赖](#依赖项目)后，使用Cmake构建项目即可。支持MinGW和Linux GNU编译器。

## 使用方法

**你的终端应支持ANSI颜色序列转义！**

从终端中打开`backup`或`restore`，使用`-h`可以查看相关帮助。

## 贡献

欢迎贡献代码、报告问题或提出改进建议。

## 许可证

该项目采用 [MIT License](https://opensource.org/licenses/MIT) 许可证。请查看 LICENSE 文件以获取更多信息。
