#!/bin/bash

# Docker 测试脚本

set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
IMAGE_NAME="diskas-builder:latest"

echo "======================================"
echo "DiskAS Docker 测试"
echo "======================================"
echo ""

# 测试 1: 版本信息
echo "[测试 1] 版本信息"
docker run --rm \
    -v "${PROJECT_DIR}:/workspace/DiskAS" \
    -w /workspace/DiskAS/build-linux \
    ${IMAGE_NAME} \
    ./DiskAS --version
echo ""

# 测试 2: 帮助信息
echo "[测试 2] 帮助信息"
docker run --rm \
    -v "${PROJECT_DIR}:/workspace/DiskAS" \
    -w /workspace/DiskAS/build-linux \
    ${IMAGE_NAME} \
    ./DiskAS --help | head -n 15
echo ""

# 测试 3: 创建测试镜像
echo "[测试 3] 创建测试镜像"
docker run --rm \
    -v "${PROJECT_DIR}:/workspace/DiskAS" \
    -w /workspace/DiskAS \
    ${IMAGE_NAME} \
    bash -c "
dd if=/dev/zero of=test_docker.img bs=1M count=5 status=none &&
printf '\xFF\xD8\xFF\xE0JFIF' | dd of=test_docker.img bs=1 seek=2048 conv=notrunc status=none &&
printf '\x89PNG\r\n\x1a\n' | dd of=test_docker.img bs=1 seek=10240 conv=notrunc status=none &&
echo '✓ Created test image with JPEG and PNG signatures'
"
echo ""

# 测试 4: 显示设备信息
echo "[测试 4] 设备信息"
docker run --rm \
    -v "${PROJECT_DIR}:/workspace/DiskAS" \
    -w /workspace/DiskAS/build-linux \
    ${IMAGE_NAME} \
    ./DiskAS -i ../test_docker.img | grep -A 5 "设备信息"
echo ""

# 测试 5: 深度扫描（仅列出）
echo "[测试 5] 深度扫描（仅列出文件）"
docker run --rm \
    -v "${PROJECT_DIR}:/workspace/DiskAS" \
    -w /workspace/DiskAS/build-linux \
    ${IMAGE_NAME} \
    ./DiskAS -m deep -l ../test_docker.img | grep -A 10 "找到.*个可恢复"
echo ""

echo "======================================"
echo "测试完成！"
echo "======================================"
echo ""
echo "Linux 版本工作正常！"
echo ""
echo "清理测试文件:"
echo "  rm test_docker.img"

