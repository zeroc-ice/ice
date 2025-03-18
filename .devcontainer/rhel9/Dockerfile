FROM registry.access.redhat.com/ubi9

RUN dnf install -y https://zeroc.com/download/ice/3.7/el9/ice-repo-3.7.el9.noarch.rpm && \
    dnf clean all

# Install required build tools and dependencies
RUN dnf install -y \
    sudo \
    make \
    curl-minimal \
    git \
    gcc-c++ \
    glibc-devel \
    libstdc++-devel \
    redhat-rpm-config \
    glibc-static \
    libstdc++-static \
    mcpp-devel \
    bzip2-devel \
    openssl-devel \
    expat-devel \
    libedit-devel \
    lmdb-devel \
    python3-devel \
    && dnf clean all

# Create a non-root user and set permissions
RUN useradd -ms /bin/bash vscode \
    && mkdir -p /home/vscode \
    && chown -R vscode:vscode /home/vscode \
    && echo "vscode ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/vscode

USER vscode
WORKDIR /home/vscode

ENV LANG=en_US.UTF-8
