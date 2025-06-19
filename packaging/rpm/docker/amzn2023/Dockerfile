# Use the official Amazon Linux 2023 image
FROM public.ecr.aws/amazonlinux/amazonlinux:2023

# Copy Ice spec file into the container
COPY packaging/rpm/ice.spec /usr/src/ice.spec

# Install required build tools and dependencies
RUN dnf config-manager --set-enabled codeready-builder-for-rhel-9-$(arch)-rpms && \
    dnf install -y \
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
    dnf builddep /usr/src/ice.spec && \
    dnf clean all
