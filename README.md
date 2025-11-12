# DiskAS - 磁盘文件恢复软件

一个功能强大、模块化的C语言硬盘文件恢复工具，支持多种文件系统和文件格式。

## 功能特性

### 核心功能
- ✅ **磁盘I/O操作**: 支持块设备和磁盘镜像文件
- ✅ **文件系统分析**: 支持 FAT12/16/32、NTFS、EXT2/3/4
- ✅ **文件签名识别**: 基于魔数识别20+种文件类型
- ✅ **智能扫描**: 快速扫描（文件系统）和深度扫描（签名）
- ✅ **文件恢复**: 批量恢复、进度显示、完整性验证

### 支持的文件类型
- **图片**: JPEG, PNG, GIF, BMP
- **文档**: PDF, DOC, DOCX, XLS, XLSX, PPT, PPTX
- **压缩**: ZIP, RAR, 7Z
- **音视频**: MP3, MP4, AVI, MOV
- **文本**: TXT, HTML, XML
- **可执行**: EXE, DLL

## 项目结构

```
DiskAS/
├── include/              # 头文件目录
│   ├── disk_io.h        # 磁盘I/O接口
│   ├── signature.h      # 文件签名识别
│   ├── file_system.h    # 文件系统分析
│   ├── scanner.h        # 磁盘扫描器
│   ├── recovery.h       # 文件恢复
│   └── utils.h          # 工具函数
├── src/                 # 源文件目录
│   ├── disk_io.c
│   ├── signature.c
│   ├── file_system.c
│   ├── scanner.c
│   ├── recovery.c
│   └── utils.c
├── main.c               # 主程序
├── CMakeLists.txt       # CMake构建文件
└── README.md            # 项目文档
```

## 快速开始

### macOS 编译

```bash
# 方式1: 使用 Makefile（推荐）
make
./DiskAS --help

# 方式2: 使用 CMake
mkdir build && cd build
cmake .. && make
./DiskAS --help
```

### Linux 编译（使用 Docker）

```bash
# 一键编译
./docker/docker-build.sh

# 测试
./docker/docker-test.sh

# 查看结果
ls -lh build-linux/DiskAS
```

### 依赖要求
- C11 编译器（GCC 或 Clang）
- Make 或 CMake
- Docker（Linux 编译）

## 使用方法

### 基本语法

```bash
./DiskAS [选项] <设备路径>
```

### 命令行选项

| 选项 | 说明 |
|------|------|
| `-h, --help` | 显示帮助信息 |
| `-v, --version` | 显示版本信息 |
| `-i, --info` | 显示设备和文件系统信息 |
| `-m, --mode <模式>` | 扫描模式: quick(快速), deep(深度), auto(自动) |
| `-o, --output <目录>` | 恢复文件的输出目录 (默认: ./recovered) |
| `-l, --list` | 仅列出可恢复的文件 |
| `-r, --recover` | 自动恢复所有找到的文件 |
| `-V, --verify` | 验证恢复的文件完整性 |

### 使用示例

#### 1. 查看设备信息
```bash
./DiskAS -i /dev/sdb1
```

#### 2. 列出可恢复的文件（不恢复）
```bash
./DiskAS -l disk_image.img
```

#### 3. 快速扫描并恢复
```bash
./DiskAS -m quick -r -o ./output /dev/sdb1
```

#### 4. 深度扫描并验证
```bash
./DiskAS -m deep -r -V disk_image.img
```

#### 5. 自动模式（推荐）
```bash
./DiskAS -r -o ./recovered disk_image.img
```

## 扫描模式说明

### 快速扫描 (Quick Scan)
- 基于文件系统元数据
- 速度快，适合最近删除的文件
- 支持 FAT 文件系统

### 深度扫描 (Deep Scan)
- 基于文件签名识别
- 全盘扫描，耗时较长
- 可恢复被覆盖的文件系统数据

### 自动模式 (Auto)
- 先快速扫描，如果没找到则深度扫描
- 平衡速度和恢复率

## 安全建议

⚠️ **重要提示**:

1. **使用磁盘镜像**: 建议先创建磁盘镜像再进行恢复
   ```bash
   # 创建磁盘镜像
   dd if=/dev/sdb of=disk_image.img bs=1M status=progress
   
   # 在镜像上进行恢复
   ./DiskAS -r disk_image.img
   ```

2. **权限要求**: 读取块设备需要 root 权限
   ```bash
   sudo ./DiskAS -r /dev/sdb1
   ```

3. **避免写入**: 程序仅读取磁盘，不会写入原设备
4. **立即停止使用**: 发现文件丢失后，立即停止使用该设备

## 模块说明

### disk_io - 磁盘I/O模块
负责底层磁盘访问，支持块设备和普通文件。

### signature - 文件签名识别模块
通过文件头魔数识别文件类型，支持20+种常见格式。

### file_system - 文件系统分析模块
解析文件系统结构，提取已删除文件的元数据。

### scanner - 磁盘扫描模块
提供快速和深度两种扫描模式，查找可恢复的文件。

### recovery - 文件恢复模块
执行文件恢复操作，支持批量处理和完整性验证。

### utils - 工具函数模块
提供辅助功能，如进度显示、文件名生成等。

## 📚 文档

- **[README.md](README.md)** - 项目主文档（你正在阅读）
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - 架构设计文档
- **[docs/DOCKER_BUILD.md](docs/DOCKER_BUILD.md)** - Docker 编译指南

## 🐳 Docker 文件

所有 Docker 相关文件位于 `docker/` 目录：
- `Dockerfile` - Docker 镜像定义
- `docker-build.sh` - 编译脚本
- `docker-shell.sh` - 进入容器
- `docker-test.sh` - 运行测试
- `docker-clean.sh` - 清理资源

## 开发计划

- [ ] 支持更多文件系统 (APFS, Btrfs, ZFS)
- [ ] 图形界面 (GUI)
- [ ] 分区表恢复
- [ ] RAID 支持
- [ ] 文件预览功能
- [ ] 恢复进度保存/恢复

## 许可证

本项目采用 MIT 许可证。

## 作者

DiskAS 项目

## 贡献

欢迎提交 Issue 和 Pull Request！

## 免责声明

本软件仅供学习和研究使用。使用本软件恢复数据时，作者不对数据丢失或损坏负责。请务必先备份重要数据。

