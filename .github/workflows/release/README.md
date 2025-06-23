# Release Workflows

This directory contains all GitHub Actions workflows used to build and publish Ice release packages.

## Workflow Index

| Workflow File                     | Description                                                   |
|-----------------------------------|---------------------------------------------------------------|
| `build-brew-packages.yml`         | Builds and uploads macOS Homebrew formulae.                   |
| `build-container-images.yml`      | Builds and pushes Docker container images to GHCR.            |
| `build-cpp-windows-packages.yml`  | Builds Windows MSI/ZIP installers for C++ packages.           |
| `build-deb-packages.yml`          | Builds `.deb` packages for Debian/Ubuntu.                     |
| `build-dotnet-packages.yml`       | Builds and publishes NuGet packages.                          |
| `build-gem-packages.yml`          | Builds and publishes RubyGems.                                |
| `build-matlab-packages.yml`       | Builds MATLAB toolbox packages.                               |
| `build-maven-packages.yml`        | Builds and publishes Maven artifacts.                         |
| `build-npm-packages.yml`          | Builds and publishes NPM packages.                            |
| `build-pip-packages.yml`          | Builds and publishes PyPI packages.                           |
| `build-rpm-packages.yml`          | Builds RPM packages for supported distributions.              |
| `build-rpm-ice-repo-packages.yml` | Builds `.repo` packages to configure YUM repos.               |
| `build-swift-package.yml`         | Builds and publishes Swift packages.                          |
| `build-xcframework-packages.yml`  | Builds iOS/macOS XCFramework packages.                        |
| `build-nightly-release.yml`       | Coordinates the full nightly release process.                 |
| `schedule-nightly-release.yml`    | Triggers the nightly release on a cron schedule.              |
| `publish-release.yml`             | Published the build artifacst to the corresponding repository |
