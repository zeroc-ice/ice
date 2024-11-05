FROM mcr.microsoft.com/devcontainers/base:ubuntu-24.04

USER vscode
WORKDIR /home/vscode

RUN set -eux \
    && wget -q https://packages.microsoft.com/config/ubuntu/24.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb \
    && sudo dpkg -i packages-microsoft-prod.deb \
    && rm packages-microsoft-prod.deb \
    && sudo apt update \
    && sudo apt-get install -y php-dev python3 python3-dev python3-passlib ruby-full gdb \
    && sudo apt-get install -y libbluetooth-dev libbz2-dev libdbus-1-dev libedit-dev libexpat1-dev liblmdb-dev libmcpp-dev libssl-dev libsystemd-dev \
    && sudo apt-get install -y openjdk-17-jdk \
    && sudo rm -rf /var/lib/apt/lists/* \
    && sudo apt-get clean

# Install .NET 8.0
RUN set -eux \
    && curl https://dot.net/v1/dotnet-install.sh -o dotnet-install.sh -L \
    && chmod +x dotnet-install.sh \
    && ./dotnet-install.sh -v latest -c 8.0

# Set environment variables
ENV DOTNET_ROOT=/home/vscode/.dotnet
ENV PATH=$DOTNET_ROOT:$PATH
ENV LANG=en_US.UTF-8
