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

- Fixed Ice for Ruby to correctly marshal an empty `sequence<byte>` supplied as a string: it now writes a single size
  byte instead of a raw 4-byte integer, which previously desynchronized the rest of the message.

[Ice 3.8.2]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
