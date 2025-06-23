# ZeroC Ice RPM Repo Builder

This Docker image provides a minimal, RHEL-compatible environment for:

- Signing RPM packages using GPG
- Generating signed YUM repositories using `createrepo_c`

## Image Name

`ghcr.io/zeroc-ice/rpm-repo-builder`

## Included Tools

- rpm-sign
- createrepo_c
- gnupg2
- rpmdevtools
