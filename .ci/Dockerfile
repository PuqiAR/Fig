FROM ubuntu:24.04  # 改用 Ubuntu 24.04，自带 GCC 13

# 设置非root用户
RUN useradd -m -s /bin/bash builder

# 1. 安装基础工具（Ubuntu 24.04 已有 GCC 13）
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        wget tar git curl ca-certificates \
        build-essential \
        && rm -rf /var/lib/apt/lists/*

# 2. 切换到builder用户安装xmake
USER builder
WORKDIR /home/builder

# 下载并安装xmake
RUN wget -O xmake https://git.fig-lang.cn/PuqiAR/xmake-binary-copy/raw/commit/989d1f2dabb0bc8d5981a5f900c2cf7c2ac78ee4/xmake-bundle-v3.0.5.linux.x86_64 && \
    chmod +x xmake && \
    ./xmake --version 2>&1 | head -1

USER root
RUN cp /home/builder/xmake /usr/local/bin/xmake && \
    chmod +x /usr/local/bin/xmake

# 3. 验证编译器版本
RUN gcc --version && g++ --version

USER builder