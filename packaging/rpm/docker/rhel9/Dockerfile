FROM registry.access.redhat.com/ubi9

# Enable ZeroC third-party repository for mcpp-devel
RUN curl -o /etc/yum.repos.d/zeroc-thirdparty.repo https://builds.testing.zeroc.com/public/dist/unstable/thirdparty/el9/zeroc-thirdparty.repo

RUN dnf install rpmdevtools make git gcc-c++ glibc-devel libstdc++-devel redhat-rpm-config glibc-static libstdc++-static -y --setopt=multilib_policy=all
