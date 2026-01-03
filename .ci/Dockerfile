FROM ubuntu:22.04

# 设置非root用户
RUN useradd -m -s /bin/bash builder

# 一次性安装所有构建所需的工具和依赖
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        wget tar git curl ca-certificates \
        build-essential make gcc g++ \
        && rm -rf /var/lib/apt/lists/*

# 切换到builder用户，在其目录下安装xmake
USER builder
WORKDIR /home/builder

# 下载并安装xmake
RUN wget -O xmake https://git.fig-lang.cn/PuqiAR/xmake-binary-copy/raw/commit/989d1f2dabb0bc8d5981a5f900c2cf7c2ac78ee4/xmake-bundle-v3.0.5.linux.x86_64 && \
    chmod +x xmake && \
    # 将xmake安装到用户目录，避免root权限问题
    ./xmake --version 2>&1 | head -1
# (使用bundle版运行一次即完成安装)

USER root
# 将xmake放入系统PATH，使所有用户可用
RUN cp /home/builder/xmake /usr/local/bin/xmake && \
    chmod +x /usr/local/bin/xmake

# 切换回builder用户作为容器默认用户
USER builder