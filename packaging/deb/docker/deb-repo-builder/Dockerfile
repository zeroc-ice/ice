FROM debian:bookworm

RUN apt-get update \
    && apt-get install -y reprepro gnupg2 \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

COPY packaging/deb/create-deb-repo.sh /usr/local/bin/create-deb-repo
RUN chmod +x /usr/local/bin/create-deb-repo
