FROM debian:trixie

# Install dependencies
RUN apt-get update && apt-get install -y \
    php-dev python3 python3-dev python3-passlib ruby-full gdb \
    libbluetooth-dev libbz2-dev libdbus-1-dev libedit-dev libexpat1-dev liblmdb-dev \
    libmcpp-dev libssl-dev libsystemd-dev \
    dh-exec dh-php dh-python locales-all php-all-dev python3-setuptools \
    javahelper maven-repo-helper \
    default-jdk gradle gradle-ice-builder-plugin libjgoodies-forms-java libjgoodies-looks-java \
    build-essential git devscripts curl \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean
