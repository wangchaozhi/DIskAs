# Docker 编译指南

本文档介绍如何使用 Docker 编译 Linux 版本的 DiskAS。

## 🎯 为什么使用 Docker？

- ✅ **跨平台编译**: 在 macOS 上编译 Linux 版本
- ✅ **环境一致性**: 确保所有人使用相同的编译环境
- ✅ **隔离性**: 不污染本地开发环境
- ✅ **可复现**: 编译结果完全一致

---

## 📋 前提条件

### 安装 Docker Desktop

#### macOS
```bash
# 使用 Homebrew
brew install --cask docker

# 或从官网下载
# https://www.docker.com/products/docker-desktop
```

启动 Docker Desktop 应用程序。

#### 验证安装
```bash
docker --version
docker info
```

---

## 🚀 快速开始

### 1. 构建 Linux 版本（一键完成）

```bash
cd /Users/wangchaozhi/CLionProjects/DiskAS
./docker-build.sh
```

这个脚本会：
1. 构建 Docker 镜像（包含所有编译工具）
2. 清理旧的编译产物
3. 在容器中编译 Linux 版本
4. 保存 Docker 镜像供以后使用

### 2. 查看编译结果

```bash
ls -lh build-linux/DiskAS
file build-linux/DiskAS
```

输出示例：
```
-rwxr-xr-x 1 root root 43K Nov 12 14:36 build-linux/DiskAS
build-linux/DiskAS: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV)
```

---

## 📁 文件说明

| 文件 | 说明 |
|------|------|
| `Dockerfile` | Docker 镜像定义 |
| `docker-build.sh` | 编译脚本 |
| `docker-shell.sh` | 进入开发容器 |
| `docker-clean.sh` | 清理 Docker 资源 |
| `.dockerignore` | Docker 忽略文件列表 |
| `diskas-builder-image.tar` | 保存的 Docker 镜像（自动生成） |
| `build-linux/` | Linux 编译产物目录 |

---

## 🔧 详细用法

### 构建并编译

```bash
./docker-build.sh
```

脚本输出：
```
======================================
DiskAS Docker 编译脚本
======================================

[INFO] Docker 环境检查通过

[INFO] 步骤 1/4: 构建 Docker 镜像...
[INFO] Docker 镜像构建成功: diskas-builder:latest

[INFO] 步骤 2/4: 清理旧的编译产物...
[INFO] 清理完成

[INFO] 步骤 3/4: 在 Docker 容器中编译项目...
=== 开始编译 Linux 版本 ===
...
[100%] Built target DiskAS
=== 编译完成 ===

[INFO] 步骤 4/4: 保存 Docker 镜像...
[INFO] Docker 镜像已保存到: diskas-builder-image.tar (XXX MB)

======================================
编译成功！
======================================

编译产物位置:
  Linux 版本: .../build-linux/DiskAS
```

### 进入开发容器

进入交互式容器进行调试或手动编译：

```bash
./docker-shell.sh
```

在容器内：
```bash
# 你现在在 Ubuntu 22.04 容器中
root@xxx:/workspace/DiskAS#

# 手动编译
cd build-linux
cmake ..
make

# 运行程序（在容器内）
./DiskAS --help

# 退出容器
exit
```

### 清理资源

```bash
./docker-clean.sh
```

会清理：
- `build-linux/` 目录
- `diskas-builder-image.tar` 文件
- Docker 镜像（可选）

---

## 🐳 Docker 镜像详情

### 基础信息

- **基础镜像**: Ubuntu 22.04
- **编译器**: GCC 11.4.0
- **构建工具**: Make 4.3, CMake 3.22.1
- **其他工具**: Git, Vim

### 查看镜像

```bash
# 列出本地镜像
docker images | grep diskas-builder

# 查看镜像详情
docker image inspect diskas-builder:latest

# 查看镜像大小
docker images diskas-builder:latest
```

### 镜像管理

```bash
# 保存镜像到文件
docker save diskas-builder:latest -o diskas-builder-image.tar

# 从文件加载镜像
docker load -i diskas-builder-image.tar

# 删除镜像
docker rmi diskas-builder:latest

# 清理未使用的镜像
docker image prune
```

---

## 🔄 在其他机器上使用

### 方法 1: 使用保存的镜像文件

在原机器上：
```bash
# 编译并保存镜像
./docker-build.sh

# 复制这两个文件到目标机器
# - diskas-builder-image.tar
# - docker-build.sh (或整个项目)
```

在目标机器上：
```bash
# 加载镜像
docker load -i diskas-builder-image.tar

# 编译项目
./docker-build.sh
```

### 方法 2: 直接使用 Dockerfile

在目标机器上：
```bash
# 复制整个项目目录
# 然后直接运行
./docker-build.sh
```

Docker 会自动构建镜像并编译项目。

---

## 📊 编译对比

### macOS 本地编译 vs Docker Linux 编译

| 特性 | macOS 本地 | Docker Linux |
|------|-----------|--------------|
| 目标平台 | macOS (ARM64/x86_64) | Linux (ARM64) |
| 可执行文件 | Mach-O | ELF |
| 文件大小 | ~70KB | ~43KB |
| 依赖库 | macOS 系统库 | Linux 系统库 |
| 运行环境 | macOS | Linux 服务器 |

### 查看差异

```bash
# macOS 版本
file build/DiskAS
# build/DiskAS: Mach-O 64-bit executable arm64

# Linux 版本
file build-linux/DiskAS
# build-linux/DiskAS: ELF 64-bit LSB executable, ARM aarch64
```

---

## 🧪 测试 Linux 版本

### 在 Docker 容器中测试

```bash
# 进入容器
./docker-shell.sh

# 在容器内运行
cd build-linux
./DiskAS --version
./DiskAS --help

# 创建测试文件
dd if=/dev/zero of=test.img bs=1M count=10

# 测试扫描
./DiskAS -i test.img
```

### 在 Linux 服务器上测试

```bash
# 将编译好的文件复制到 Linux 服务器
scp build-linux/DiskAS user@linux-server:/tmp/

# SSH 到服务器
ssh user@linux-server

# 运行程序
chmod +x /tmp/DiskAS
/tmp/DiskAS --version
```

---

## 🎨 自定义 Docker 镜像

### 修改 Dockerfile

编辑 `Dockerfile` 添加更多工具：

```dockerfile
# 添加其他工具
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    make \
    cmake \
    git \
    vim \
    gdb \         # 添加调试器
    valgrind \    # 添加内存检查工具
    strace \      # 添加系统调用跟踪
    && rm -rf /var/lib/apt/lists/*
```

### 使用不同的基础镜像

```dockerfile
# 使用其他 Linux 发行版
FROM debian:11
FROM centos:8
FROM alpine:latest
```

### 构建不同架构

```bash
# x86_64 架构
docker buildx build --platform linux/amd64 -t diskas-builder:amd64 .

# ARM64 架构（默认）
docker buildx build --platform linux/arm64 -t diskas-builder:arm64 .
```

---

## 📝 常见问题

### Q1: Docker Desktop 没有运行？

**错误信息**:
```
Cannot connect to the Docker daemon
```

**解决方案**:
```bash
# 启动 Docker Desktop 应用程序
open -a Docker

# 等待几秒钟，然后验证
docker info
```

### Q2: 镜像构建很慢？

**原因**: 下载基础镜像和软件包需要时间

**优化方案**:
```bash
# 使用国内镜像源（修改 Dockerfile）
RUN sed -i 's/ports.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    apt-get update && apt-get install -y ...
```

### Q3: 编译时出现权限错误？

**原因**: 容器内以 root 身份运行，生成的文件属于 root

**解决方案**:
```bash
# 修改文件所有权
sudo chown -R $(whoami) build-linux/
```

### Q4: 想在 Docker 中调试程序？

**方案**:
```bash
# 进入容器
./docker-shell.sh

# 使用 gdb 调试
cd build-linux
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
gdb ./DiskAS
```

### Q5: 如何编译 x86_64 版本？

**方案**: 修改 Dockerfile 第一行
```dockerfile
FROM --platform=linux/amd64 ubuntu:22.04
```

然后重新运行：
```bash
./docker-build.sh
```

### Q6: 磁盘空间不足？

**清理方案**:
```bash
# 清理所有未使用的 Docker 资源
docker system prune -a

# 清理编译产物
./docker-clean.sh
```

---

## 🔒 安全注意事项

1. **只读挂载**: Docker 挂载是读写的，容器可以修改项目文件
2. **网络访问**: 容器在构建时需要网络下载软件包
3. **权限问题**: 容器内以 root 运行，生成的文件需要修改权限

---

## 📈 性能优化

### 多核编译

修改 `docker-build.sh` 中的编译命令：

```bash
# 原始
cmake --build .

# 优化（使用所有 CPU 核心）
cmake --build . -j$(nproc)
```

### 缓存优化

Docker 会缓存镜像层，修改 Dockerfile 时：
- 频繁变化的放在后面
- 不变的放在前面

### 减小镜像体积

```dockerfile
# 使用 alpine 替代 ubuntu（更小）
FROM alpine:latest

# 或清理缓存
RUN apt-get update && apt-get install -y ... \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
```

---

## 🚢 CI/CD 集成

### GitHub Actions 示例

```yaml
name: Build Linux Version

on: [push]

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Build with Docker
        run: ./docker-build.sh
      - name: Upload artifact
        uses: actions/upload-artifact@v2
        with:
          name: diskas-linux
          path: build-linux/DiskAS
```

---

## 📚 相关资源

- **Docker 官方文档**: https://docs.docker.com/
- **Docker Hub**: https://hub.docker.com/
- **CMake 文档**: https://cmake.org/documentation/
- **Ubuntu 镜像**: https://hub.docker.com/_/ubuntu

---

## ✅ 总结

使用 Docker 编译 Linux 版本的优势：

1. ✅ **跨平台**: 在 macOS 上编译 Linux 版本
2. ✅ **环境隔离**: 不影响本地环境
3. ✅ **可重现**: 每次编译结果一致
4. ✅ **易分享**: 镜像文件可以分享给团队
5. ✅ **CI/CD 友好**: 易于集成到自动化流程

现在你有两种编译方式：
- **macOS 本地**: 使用 `make` 或 `cmake`（开发调试）
- **Docker Linux**: 使用 `./docker-build.sh`（发布部署）

Happy Building! 🎉

