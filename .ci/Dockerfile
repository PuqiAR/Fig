FROM ubuntu:22.04

# 设置非root用户
RUN useradd -m -s /bin/bash builder

# 1. 安装构建工具和编译器（包含 GCC 13）
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        wget tar git curl ca-certificates \
        build-essential \
        gcc-13 g++-13 \
        && rm -rf /var/lib/apt/lists/*

# 2. 设置 GCC 13 为默认编译器
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100

# 3. 切换到builder用户安装xmake
USER builder
WORKDIR /home/builder

# 下载并安装xmake
RUN wget -O xmake https://git.fig-lang.cn/PuqiAR/xmake-binary-copy/raw/commit/989d1f2dabb0bc8d5981a5f900c2cf7c2ac78ee4/xmake-bundle-v3.0.5.linux.x86_64 && \
    chmod +x xmake && \
    ./xmake --version 2>&1 | head -1

USER root
RUN cp /home/builder/xmake /usr/local/bin/xmake && \
    chmod +x /usr/local/bin/xmake

# 4. 验证编译器版本
RUN gcc --version && g++ --version

USER builder