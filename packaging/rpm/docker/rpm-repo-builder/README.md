# ZeroC Ice RPM Repo Builder

This Docker image provides a minimal, RHEL-compatible environment for:

- Signing RPM packages using GPG
- Generating signed YUM repositories using `createrepo_c`

## Image Name

`ghcr.io/zeroc-ice/rpm-repo-builder-<channel>`

Where `<channel>` is the Ice version channel (e.g., `3.8`, `3.9`).

## Included Tools

- rpm-sign
- createrepo_c
- gnupg2
- rpmdevtools
