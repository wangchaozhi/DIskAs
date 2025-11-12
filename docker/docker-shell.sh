#!/bin/bash

# 进入 DiskAS 开发容器的交互式 Shell

set -e

IMAGE_NAME="diskas-builder"
IMAGE_TAG="latest"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

echo "======================================"
echo "进入 DiskAS 开发容器"
echo "======================================"
echo ""

# 检查镜像是否存在
if ! docker image inspect ${IMAGE_NAME}:${IMAGE_TAG} &> /dev/null; then
    echo "Docker 镜像不存在，正在构建..."
    docker build -t ${IMAGE_NAME}:${IMAGE_TAG} -f docker/Dockerfile .
fi

echo "启动容器..."
echo "项目目录: /workspace/DiskAS"
echo "退出容器: 输入 exit 或按 Ctrl+D"
echo ""

# 启动交互式容器
docker run -it --rm \
    -v "${PROJECT_DIR}:/workspace/DiskAS" \
    -w /workspace/DiskAS \
    ${IMAGE_NAME}:${IMAGE_TAG} \
    /bin/bash

