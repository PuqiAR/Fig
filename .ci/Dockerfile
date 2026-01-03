FROM ubuntu:24.04

# 1. 安装LLVM/Clang 17+工具链（Ubuntu 24.04官方源已包含）
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        wget tar git curl ca-certificates \
        build-essential \
        clang-17 lld-17 libc++-17-dev libc++abi-17-dev \
        && rm -rf /var/lib/apt/lists/* && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-17 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-17 100 && \
    update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-17 100

# 2. 设置非root用户
RUN useradd -m -s /bin/bash builder

# 3. 切换到builder用户安装xmake
USER builder
WORKDIR /home/builder

RUN wget -O xmake https://git.fig-lang.cn/PuqiAR/xmake-binary-copy/raw/commit/989d1f2dabb0bc8d5981a5f900c2cf7c2ac78ee4/xmake-bundle-v3.0.5.linux.x86_64 && \
    chmod +x xmake && \
    ./xmake --version 2>&1 | head -1

# 4. 切回root，将xmake安装到系统路径
USER root
RUN cp /home/builder/xmake /usr/local/bin/xmake && \
    chmod +x /usr/local/bin/xmake

# # 5. 验证环境
# RUN clang++ --version && xmake --version

USER builder