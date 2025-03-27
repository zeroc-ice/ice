FROM mcr.microsoft.com/devcontainers/base:debian-12

USER vscode
WORKDIR /home/vscode

RUN sudo apt update \
    && sudo apt-get install -y \
    php-dev python3 python3-dev python3-passlib ruby-full gdb \
    libbluetooth-dev libbz2-dev libdbus-1-dev libedit-dev libexpat1-dev liblmdb-dev \
    libmcpp-dev libssl-dev libsystemd-dev \
    dh-exec dh-php dh-python locales-all php-all-dev python3-setuptools \
    build-essential git devscripts curl \
    && sudo rm -rf /var/lib/apt/lists/* \
    && sudo apt-get clean
