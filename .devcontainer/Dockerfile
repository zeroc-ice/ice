FROM mcr.microsoft.com/devcontainers/base:bookworm

USER vscode
WORKDIR /home/vscode

# Install necessary packages
RUN set -eux \
    && wget -q https://packages.microsoft.com/config/ubuntu/22.04/packages-microsoft-prod.deb -O packages-microsoft-prod.deb \
    && sudo dpkg -i packages-microsoft-prod.deb \
    && rm packages-microsoft-prod.deb \
    && sudo apt update \
    && sudo apt-get install -y libbluetooth-dev libbz2-dev libdbus-1-dev libedit-dev libexpat1-dev liblmdb-dev libmcpp-dev libssl-dev libsystemd-dev \
    && sudo rm -rf /var/lib/apt/lists/* \
    && sudo apt-get clean
