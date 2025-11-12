#!/bin/bash

# 清理 Docker 相关资源

set -e

IMAGE_NAME="diskas-builder"
IMAGE_TAG="latest"

echo "======================================"
echo "清理 Docker 资源"
echo "======================================"
echo ""

# 删除构建产物
if [ -d "build-linux" ]; then
    echo "删除 build-linux 目录..."
    rm -rf build-linux
    echo "✓ 已删除"
fi

# 删除镜像文件
if [ -f "diskas-builder-image.tar" ]; then
    echo "删除镜像文件..."
    rm -f diskas-builder-image.tar
    echo "✓ 已删除"
fi

# 询问是否删除 Docker 镜像
read -p "是否删除 Docker 镜像 ${IMAGE_NAME}:${IMAGE_TAG}? (y/n) " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    if docker image inspect ${IMAGE_NAME}:${IMAGE_TAG} &> /dev/null; then
        docker rmi ${IMAGE_NAME}:${IMAGE_TAG}
        echo "✓ Docker 镜像已删除"
    else
        echo "镜像不存在"
    fi
fi

echo ""
echo "清理完成！"

