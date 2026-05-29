# Ice 3.9 Changelog <!-- omit in toc -->

The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

- [Changes in Ice 3.9.0](#changes-in-ice-390)
  - [Ice Service Changes](#ice-service-changes)

## Changes in Ice 3.9.0

These are the changes since the [Ice 3.8.2] release.

### Ice Service Changes

#### DataStorm

- Fixed an issue where a relay could lose topic announcements when a relay-to-relay session was established after the
  announcements were received. Relay nodes now replay previously received topic announcements when such a session is
  established or re-established, preventing discovery deadlocks between applications connected to different relay nodes.
  (#5359)

[Ice 3.8.2]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
