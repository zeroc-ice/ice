FROM registry.access.redhat.com/ubi9

# Enable Ice 3.7 repository for third-party dependencies (mcpp-devel, lmdb-devel).
RUN dnf install -y https://zeroc.com/download/ice/3.7/el9/ice-repo-3.7.el9.noarch.rpm && \
    dnf clean all

# Install required build tools and dependencies
RUN dnf install --setopt=multilib_policy=all -y \
    rpmdevtools \
    make \
    curl-minimal \
    git \
    gcc-c++ \
    glibc-devel \
    libstdc++-devel \
    redhat-rpm-config \
    glibc-static \
    libstdc++-static && \
    dnf clean all

# Copy Ice spec file into the container
COPY packaging/rpm/ice.spec /usr/src/ice.spec

# Install build dependencies. Do not use dnf builddep as it doesn't obey multilib_policy=all.
RUN rpmspec --parse /usr/src/ice.spec 2>/dev/null | \
    grep '^BuildRequires:' | \
    sed -E 's/BuildRequires:\s*//' | \
    tr ',' '\n' | awk '{print $1}' | sort -u > /tmp/dependencies.txt && \
    dnf install --setopt=multilib_policy=all -y $(cat /tmp/dependencies.txt) && \
    rm -f /tmp/dependencies.txt /usr/src/ice.spec && \
    dnf clean all
