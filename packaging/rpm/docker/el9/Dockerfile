FROM registry.access.redhat.com/ubi9

# Copy Ice spec file into the container
COPY packaging/rpm/ice.spec /usr/src/ice.spec

# Install required tools and build dependencies:
# - Enable the Ice 3.8 repository to install mcpp-devel
# - Register with subscription-manager to enable CodeReady Builder (needed for lmdb-devel)
# - Install build tools and dependencies
# - Unregister and remove RHSM credentials and entitlements for cleanup

RUN --mount=type=secret,id=rh_credentials \
    source /run/secrets/rh_credentials && \
    subscription-manager register --username "$RH_USERNAME" --password "$RH_PASSWORD" && \
    dnf install -y https://zeroc.com/download/ice/3.8/el9/ice-repo-3.8-1.el9.noarch.rpm && \
    dnf config-manager --set-enabled codeready-builder-for-rhel-9-$(arch)-rpms && \
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
    libstdc++-static \
    rpm-sign && \
    # Use --nobest to avoid issues with  systemd-devel-252-51.el9_6.1 dependencies not being available.
    dnf builddep -y /usr/src/ice.spec --nobest && \
    dnf clean all && \
    subscription-manager unregister && \
    rm -rf /etc/pki/entitlement /etc/rhsm/ca
