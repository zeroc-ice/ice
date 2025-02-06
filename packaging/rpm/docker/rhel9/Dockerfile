FROM registry.access.redhat.com/ubi9

# Enable Ice 3.7 repository for third-party dependencies (mcpp-devel, lmdb-devel).
RUN dnf install -y https://zeroc.com/download/ice/3.7/el9/ice-repo-3.7.el9.noarch.rpm && \
    dnf clean all

# Install required build tools and dependencies
RUN dnf install -y \
    rpmdevtools \
    make \
    git \
    gcc-c++ \
    glibc-devel \
    libstdc++-devel \
    redhat-rpm-config \
    glibc-static \
    libstdc++-static \
    --setopt=multilib_policy=all && \
    dnf clean all

# Copy Ice spec file into the container
COPY ../../ice.spec /usr/src/ice.spec

# Install build dependencies from the spec file
RUN dnf builddep -y --setopt=multilib_policy=all /usr/src/ice.spec && \
    dnf clean all && \
    rm /usr/src/ice.spec
