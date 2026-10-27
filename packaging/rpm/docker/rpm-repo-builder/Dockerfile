FROM rockylinux:9

RUN dnf install -y \
    rpmdevtools \
    createrepo_c \
    gnupg2 \
    rpm-sign && \
    dnf clean all
