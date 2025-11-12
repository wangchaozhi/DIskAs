#!/bin/bash

# DiskAS Docker 编译脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 配置
IMAGE_NAME="diskas-builder"
IMAGE_TAG="latest"
CONTAINER_NAME="diskas-build-container"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo -e "${BLUE}======================================"
echo "DiskAS Docker 编译脚本"
echo "======================================${NC}"
echo ""

# 函数：打印信息
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# 检查 Docker 是否安装
if ! command -v docker &> /dev/null; then
    print_error "Docker 未安装，请先安装 Docker"
    echo "macOS: brew install --cask docker"
    echo "或访问: https://www.docker.com/products/docker-desktop"
    exit 1
fi

# 检查 Docker 是否运行
if ! docker info &> /dev/null; then
    print_error "Docker 未运行，请启动 Docker Desktop"
    exit 1
fi

print_info "Docker 环境检查通过"
echo ""

# 步骤 1: 构建 Docker 镜像
print_info "步骤 1/4: 构建 Docker 镜像..."
if docker build -t ${IMAGE_NAME}:${IMAGE_TAG} . ; then
    print_info "Docker 镜像构建成功: ${IMAGE_NAME}:${IMAGE_TAG}"
else
    print_error "Docker 镜像构建失败"
    exit 1
fi
echo ""

# 步骤 2: 清理旧的编译产物
print_info "步骤 2/4: 清理旧的编译产物..."
rm -rf build-linux
mkdir -p build-linux
print_info "清理完成"
echo ""

# 步骤 3: 在 Docker 容器中编译
print_info "步骤 3/4: 在 Docker 容器中编译项目..."
docker run --rm \
    -v "${PROJECT_DIR}:/workspace/DiskAS" \
    -w /workspace/DiskAS \
    ${IMAGE_NAME}:${IMAGE_TAG} \
    bash -c "
        echo '=== 开始编译 Linux 版本 ===' && \
        mkdir -p build-linux && \
        cd build-linux && \
        cmake -DCMAKE_BUILD_TYPE=Release .. && \
        cmake --build . && \
        echo '' && \
        echo '=== 编译完成 ===' && \
        ls -lh DiskAS && \
        echo '' && \
        echo '=== 验证可执行文件 ===' && \
        file DiskAS && \
        ./DiskAS --version
    "

if [ $? -eq 0 ]; then
    print_info "编译成功！"
else
    print_error "编译失败"
    exit 1
fi
echo ""

# 步骤 4: 保存 Docker 镜像
print_info "步骤 4/4: 保存 Docker 镜像..."
IMAGE_FILE="diskas-builder-image.tar"
if docker save ${IMAGE_NAME}:${IMAGE_TAG} -o ${IMAGE_FILE}; then
    IMAGE_SIZE=$(du -h ${IMAGE_FILE} | cut -f1)
    print_info "Docker 镜像已保存到: ${IMAGE_FILE} (${IMAGE_SIZE})"
else
    print_warning "保存镜像失败（但编译成功）"
fi
echo ""

# 显示结果
echo -e "${GREEN}======================================"
echo "编译成功！"
echo "======================================${NC}"
echo ""
echo "编译产物位置:"
echo "  Linux 版本: ${PROJECT_DIR}/build-linux/DiskAS"
echo ""
echo "Docker 镜像信息:"
echo "  镜像名称: ${IMAGE_NAME}:${IMAGE_TAG}"
echo "  镜像文件: ${IMAGE_FILE} (可用于其他机器)"
echo ""
echo "下次编译使用:"
echo "  ./docker-build.sh"
echo ""
echo "进入开发容器:"
echo "  ./docker-shell.sh"
echo ""

# 显示文件信息
if [ -f "build-linux/DiskAS" ]; then
    echo "可执行文件信息:"
    ls -lh build-linux/DiskAS
    file build-linux/DiskAS
fi

