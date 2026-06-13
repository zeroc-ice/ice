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

- Fixed `slice2rb` to escape an operation parameter whose mapped name lower-cases to `context` (such as a parameter
  named `Context`). It previously clashed with the synthesized trailing `context` parameter of the generated proxy
  method, producing a Ruby `SyntaxError` that made the generated file unloadable.

[Ice 3.8.2]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
