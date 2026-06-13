# Ice 3.9 Changelog <!-- omit in toc -->

The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

- [Changes in Ice 3.9.0](#changes-in-ice-390)
  - [General Changes](#general-changes)

## Changes in Ice 3.9.0

These are the changes since the [Ice 3.8.2] release.

### General Changes

- Fixed Ice for Ruby to keep a custom `SliceLoader` alive for the communicator's lifetime. The wrapper previously
  registered the wrong address with the garbage collector, leaving a dangling root and allowing the loader to be
  collected while still in use.

[Ice 3.8.2]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
