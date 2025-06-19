# Use the official Amazon Linux 2023 image
FROM public.ecr.aws/amazonlinux/amazonlinux:2023

# Copy Ice spec file into the container
COPY packaging/rpm/ice.spec /usr/src/ice.spec

# Install required build tools and dependencies
RUN dnf install -y \
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
    rpmspec -q --buildrequires /usr/src/ice.spec | xargs -r dnf install -y && \
    dnf clean all
