FROM rockylinux:9

RUN dnf install -y \
    rpmdevtools \
    createrepo_c \
    gnupg2 \
    rpm-sign && \
    dnf clean all

COPY packaging/rpm/rpm-sign.sh /usr/local/bin/rpm-sign
RUN chmod +x /usr/local/bin/rpm-sign

COPY packaging/rpm/create-rpm-repo.sh /usr/local/bin/create-rpm-repo
RUN chmod +x /usr/local/bin/create-rpm-repo
