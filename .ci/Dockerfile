FROM ubuntu:24.04

# 1. 【关键优化】仅添加LLVM仓库并安装Clang 19必要组件
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        wget gnupg ca-certificates && \
    wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add - && \
    echo "deb http://apt.llvm.org/noble/ llvm-toolchain-noble-19 main" > /etc/apt/sources.list.d/llvm.list && \
    apt-get update && \
    # 只安装最核心的Clang工具链、基础工具和xmake的依赖
    apt-get install -y --no-install-recommends \
        clang-19 lld-19 libc++-19-dev libc++abi-19-dev \
        tar git curl \
        && rm -rf /var/lib/apt/lists/* && \
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-19 100 && \
    update-alternatives --install /usr/bin/clang++ clang++ /usr/bin/clang++-19 100 && \
    update-alternatives --install /usr/bin/ld ld /usr/bin/ld.lld-19 100

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

USER builder