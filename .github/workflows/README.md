# Release Workflows

The release workflows support building and publishing Ice packages for the `3.8` and `nightly` channels.

The `3.8` channel publishes packages to the stable release repositories, while the `nightly` channel publishes to the
unstable nightly repositories.

The nightly repositories are updated daily from the `zeroc-ice/ice` `main` branch. These releases are intended for testing
the latest development and must not be used in production.

To build a stable release for `3.8`, run the `build-release` workflow and pass `3.8` as the channel.
To build a dev release for `nightly`, run the `build-release` workflow and pass `nightly` as the channel.

The `build-nightly-release` workflow schedules a daily `build-release` run for the `nightly` channel.

## Implementation Notes

Each `build-xxx-package` workflow builds the corresponding `xxx` package, and each `publish-xxx-package` workflow
publishes it.

The `build-release` workflow orchestrates the build of all packages and triggers `publish-release` after all
`build-xxx-package` workflows have completed successfully.

The `publish-release` workflow orchestrates the publishing of all packages to their respective repositories by calling
the `publish-xxx-package` workflows. You can also run it manually by providing a `run_id` and `channel`; in this case,
it will publish the artifacts from the specified run to the specified channel.

GitHub allows a maximum of 20 reusable workflows, and the release pipeline is close to this limit. To avoid exceeding
it, some workflows were merged into single jobs.

For example, `build-macos-packages` builds both the `xcframework` and `brew` packages, and `publish-misc-packages` is
used to publish `msi`, `matlab`, `xcframework`, `swift`, and `brew` packages.

The RPM and DEB packaging workflows use Docker images from `packaging/rpm/docker` and `packaging/deb/docker`.

The release pipeline is configured to use `ghcr.io`. These images can be built using the `build-container-images`
workflow, which builds and publishes them to `ghcr.io` for internal use.
