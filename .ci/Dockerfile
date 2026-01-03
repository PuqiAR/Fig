FROM ubuntu:24.04

# 1. 设置镜像源（可选，用于加速）
RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list && \
    sed -i 's/security.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list

# 2. 一次性安装所有依赖（工具链、库、CI工具）
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        wget gnupg ca-certificates \
        clang-19 lld-19 libc++-19-dev libc++abi-19-dev \
        mingw-w64 g++-mingw-w64 \
        git tar curl \
        python3 python3-pip python3.12-venv \
        && rm -rf /var/lib/apt/lists/* && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100 && \
    ln -sf /usr/bin/ld.lld-19 /usr/bin/ld.lld

# 3. 安装xmake
RUN wget -O /usr/local/bin/xmake \
    https://git.fig-lang.cn/PuqiAR/xmake-binary-copy/raw/commit/989d1f2dabb0bc8d5981a5f900c2cf7c2ac78ee4/xmake-bundle-v3.0.5.linux.x86_64 && \
    chmod +x /usr/local/bin/xmake

# 4. 创建非root用户
RUN useradd -m -s /bin/bash builder
USER builder
WORKDIR /home/builder

RUN xmake --version | head -1 && \
    clang++ --version | head -1 && \
    git --version && \
    echo "✅ 核心工具就绪"