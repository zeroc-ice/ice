# Release Workflows

This directory contains all GitHub Actions workflows used to build and publish Ice release packages.

Each `build-xxx-package` workflow builds the corresponding `xxx` package, and each `publish-xxx-package` workflow
publishes it.

The `build-release` workflow orchestrates the build of all packages, and calls `publish-release` after all
`build-xxx-package` workflows have completed successfully.

The `build-nightly-release` workflow schedules a daily `build-release` run for the `nightly` channel. You can also
trigger this workflow manually to start a nightly release build.

The `build-stable-release` workflow can be triggered manually to run a stable release build from a given Git tag.

The `publish-release` workflow orchestrates the publishing of all packages to their respective repositories by calling
the `publish-xxx-package` workflows. You can also run it manually by providing a `run_id` and `channel`; in this case,
it will publish the artifacts from the specified run to the specified channel.
