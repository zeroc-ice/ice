FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

USER vscode
WORKDIR /home/vscode

# Install necessary packages
RUN set -eux \
    && wget -q https://packages.microsoft.com/config/ubuntu/24.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb \
    && sudo dpkg -i packages-microsoft-prod.deb \
    && rm packages-microsoft-prod.deb \
    && sudo apt update \
    && sudo apt-get install -y php-dev python3 python3-dev python3-passlib ruby-full gdb \
    && sudo apt-get install -y libbluetooth-dev libbz2-dev libdbus-1-dev libedit-dev libexpat1-dev liblmdb-dev libmcpp-dev libssl-dev libsystemd-dev \
    # stress is a tool to impose load on and stress test systems
    && stress \
    && sudo rm -rf /var/lib/apt/lists/* \
    && sudo apt-get clean

# Borrowed from https://github.com/oracle/docker-images/blob/main/OracleJava/17/Dockerfile
ARG JAVA_URL=https://download.oracle.com/java/17/latest
ARG JAVA_HOME=/usr/java/jdk-17

RUN set -eux; \
    ARCH="$(uname -m)" && \
    # Java uses just x64 in the name of the tarball
    if [ "$ARCH" = "x86_64" ]; \
    then ARCH="x64"; \
    fi && \
    JAVA_PKG="$JAVA_URL"/jdk-17_linux-"${ARCH}"_bin.tar.gz ; \
    JAVA_SHA256="$(curl "$JAVA_PKG".sha256)" ; \
    curl --output /tmp/jdk.tgz "$JAVA_PKG" && \
    echo "$JAVA_SHA256" */tmp/jdk.tgz | sha256sum -c; \
    sudo mkdir -p "$JAVA_HOME"; \
    sudo tar --extract --file /tmp/jdk.tgz --directory "$JAVA_HOME" --strip-components 1; \
    rm /tmp/jdk.tgz;

# Install .NET 8.0
RUN set -eux \
    && curl https://dot.net/v1/dotnet-install.sh -o dotnet-install.sh -L \
    && chmod +x dotnet-install.sh \
    && ./dotnet-install.sh -v latest -c 8.0

# Set environment variables
ENV DOTNET_ROOT=/home/vscode/.dotnet
ENV PATH=$DOTNET_ROOT:$PATH
ENV LANG=en_US.UTF-8
ENV JAVA_HOME=/usr/java/jdk-17
ENV PATH=$JAVA_HOME/bin:$PATH
