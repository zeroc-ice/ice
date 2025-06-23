FROM debian:bookworm

RUN apt-get update \
    && apt-get install -y reprepro gnupg2 \
    && rm -rf /var/lib/apt/lists/* \
    && apt-get clean

COPY packaging/dpkg/create-repo.sh /usr/local/bin/create-repo
RUN chmod +x /usr/local/bin/create-repo
