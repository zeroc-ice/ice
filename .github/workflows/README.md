# Release Workflows

The release workflows build and publish stable, release candidate, and nightly Ice packages. The release channel is
read from `config/version.env` on the branch or tag being built.

Nightly packages are intended for testing the latest development and must not be used in production. The
`dispatch-nightly-release` workflow is the central scheduler: it runs from the default branch and dispatches nightly
releases for `main` and the supported maintenance branches.

## Building and Publishing

The `build-release` workflow builds all packages and uploads their artifacts. It never publishes them. For stable and
release candidate releases, run `build-release`, make any required updates between building and publishing (such as
updating the Swift package manifest with release binary hashes), and then run `publish-release` with the build's
`run_id`, `channel`, and `quality`.

The `nightly-release` workflow orchestrates the nightly path. It always runs `build-release`; when `publish` is true,
it then prunes expired nightly artifacts and runs `publish-release`. Pruning must finish before publishing because it
can remove packages from the APT and RPM repository trees, while the publish workflows regenerate their metadata.
Run `nightly-release` with `publish` false to build nightly artifacts without changing the repositories.

The scheduler dispatches `nightly-release` for branches that contain this split workflow. Maintenance branches that
have not received the workflow yet continue to use their branch-local `build-release` orchestration.

## Implementation Notes

Each `build-xxx-package` workflow builds the corresponding package, and each `publish-xxx-package` workflow publishes
it. `publish-release` calls all package-specific publish workflows and can also be run manually to publish artifacts
from a specified build run.

The RPM and DEB packaging workflows use Docker images from `packaging/rpm/docker` and `packaging/deb/docker`. The
release pipeline uses `ghcr.io`; the `build-container-images` workflow builds and publishes these images for internal
use.
