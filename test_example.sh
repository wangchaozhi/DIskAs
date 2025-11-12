#!/bin/bash

# DiskAS 测试示例脚本
# 此脚本演示如何创建测试磁盘镜像并使用 DiskAS 进行恢复

set -e

echo "=================================="
echo "DiskAS 测试示例"
echo "=================================="
echo ""

# 1. 创建测试磁盘镜像
echo "[1] 创建测试磁盘镜像 (10MB)..."
dd if=/dev/zero of=test_disk.img bs=1M count=10 2>/dev/null
echo "    ✓ 磁盘镜像创建完成: test_disk.img"
echo ""

# 2. 格式化为 FAT32 (需要 root 权限)
echo "[2] 尝试格式化为 FAT32..."
echo "    注意: 此步骤在 macOS 上可能需要不同的命令"
echo "    跳过格式化步骤..."
echo ""

# 3. 创建一些测试文件并写入镜像
echo "[3] 创建测试文件..."
mkdir -p test_files

# 创建 JPEG 文件头（模拟）
printf '\xFF\xD8\xFF\xE0\x00\x10JFIF\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00' > test_files/test.jpg
dd if=/dev/urandom bs=1K count=50 >> test_files/test.jpg 2>/dev/null
echo "    ✓ 创建测试 JPEG 文件"

# 创建 PNG 文件头（模拟）
printf '\x89PNG\r\n\x1a\n\x00\x00\x00\rIHDR' > test_files/test.png
dd if=/dev/urandom bs=1K count=30 >> test_files/test.png 2>/dev/null
echo "    ✓ 创建测试 PNG 文件"

# 创建 PDF 文件头（模拟）
printf '%%PDF-1.4\n%%EOF' > test_files/test.pdf
dd if=/dev/urandom bs=1K count=100 >> test_files/test.pdf 2>/dev/null
echo "    ✓ 创建测试 PDF 文件"

# 创建 ZIP 文件头（模拟）
printf 'PK\x03\x04' > test_files/test.zip
dd if=/dev/urandom bs=1K count=20 >> test_files/test.zip 2>/dev/null
echo "    ✓ 创建测试 ZIP 文件"

# 将文件写入磁盘镜像
echo ""
echo "[4] 将测试文件写入磁盘镜像..."
dd if=test_files/test.jpg of=test_disk.img bs=512 seek=100 conv=notrunc 2>/dev/null
dd if=test_files/test.png of=test_disk.img bs=512 seek=300 conv=notrunc 2>/dev/null
dd if=test_files/test.pdf of=test_disk.img bs=512 seek=500 conv=notrunc 2>/dev/null
dd if=test_files/test.zip of=test_disk.img bs=512 seek=800 conv=notrunc 2>/dev/null
echo "    ✓ 测试文件已写入磁盘镜像"
echo ""

# 4. 测试 DiskAS
echo "=================================="
echo "开始测试 DiskAS"
echo "=================================="
echo ""

# 编译程序
if [ ! -f "./DiskAS" ]; then
    echo "[编译] 编译 DiskAS..."
    make clean && make
    echo ""
fi

# 测试 1: 显示版本
echo "[测试 1] 显示版本信息"
./DiskAS --version
echo ""

# 测试 2: 显示设备信息
echo "[测试 2] 显示设备信息"
./DiskAS -i test_disk.img
echo ""

# 测试 3: 深度扫描并列出文件
echo "[测试 3] 深度扫描磁盘镜像"
./DiskAS -m deep -l test_disk.img
echo ""

# 测试 4: 恢复文件
echo "[测试 4] 恢复找到的文件"
./DiskAS -m deep -r -o ./recovered test_disk.img
echo ""

# 5. 检查恢复结果
echo "=================================="
echo "检查恢复结果"
echo "=================================="
if [ -d "./recovered" ]; then
    echo "恢复的文件列表:"
    ls -lh ./recovered/
    echo ""
    echo "文件类型识别:"
    for file in ./recovered/*; do
        if [ -f "$file" ]; then
            echo "  $(basename $file): $(file $file)"
        fi
    done
else
    echo "没有找到恢复的文件"
fi
echo ""

# 6. 清理
echo "=================================="
echo "清理测试文件"
echo "=================================="
read -p "是否删除测试文件? (y/n) " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf test_disk.img test_files recovered
    echo "✓ 清理完成"
else
    echo "保留测试文件"
fi

echo ""
echo "=================================="
echo "测试完成！"
echo "=================================="

