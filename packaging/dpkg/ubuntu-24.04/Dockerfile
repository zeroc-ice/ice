FROM ubuntu:24.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    php-dev python3 python3-dev python3-passlib ruby-full gdb \
    libbluetooth-dev libbz2-dev libdbus-1-dev libedit-dev libexpat1-dev liblmdb-dev \
    libmcpp-dev libssl-dev libsystemd-dev \
    dh-exec dh-php dh-python locales-all php-all-dev python3-setuptools \
    build-essential \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

# Set working directory
WORKDIR /workspace

# Copy build script into the container
COPY build-package.sh /usr/local/bin/build-package.sh

# Make the script executable
RUN chmod +x /usr/local/bin/build-package.sh

# Set the default command to run the build script (can be overridden)
ENTRYPOINT ["/usr/local/bin/build-package.sh"]
