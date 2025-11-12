# DiskAS Linux 编译环境
FROM ubuntu:22.04

# 设置非交互模式
ENV DEBIAN_FRONTEND=noninteractive

# 设置工作目录
WORKDIR /workspace

# 更新包列表并安装编译工具
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc \
    g++ \
    make \
    cmake \
    git \
    vim \
    && rm -rf /var/lib/apt/lists/*

# 创建编译目录
RUN mkdir -p /workspace/DiskAS

# 设置环境变量
ENV CC=gcc
ENV CXX=g++
ENV PATH="/usr/local/bin:${PATH}"

# 显示版本信息
RUN gcc --version && \
    make --version && \
    cmake --version

# 设置默认命令
CMD ["/bin/bash"]

