# Ice 3.9 Changelog <!-- omit in toc -->

The entries below contain brief descriptions of the changes in each release, in no particular order. Some of the
entries reflect significant new additions, while others represent minor corrections. Although this list is not a
comprehensive report of every change we made in a release, it does provide details on the changes we feel Ice users
might need to be aware of.

- [Changes in Ice 3.9.0](#changes-in-ice-390)
  - [General Changes](#general-changes)
  - [C# Changes](#c-changes)
  - [JavaScript Changes](#javascript-changes)

## Changes in Ice 3.9.0

### General Changes

- Fixed the server-side WebSocket opening handshake to reject messages that are not `GET` requests, as required by
  RFC 6455. A peer could previously trip an assertion by sending a response-shaped handshake.

### C# Changes

- Fixed a bug in `slice2cs --icerpc` where the generated request decoder read in-parameters in declaration order
  instead of the marshal order. As a result, an operation with an optional parameter declared before a required
  parameter (or optional parameters declared out of tag order) could fail to decode or decode incorrect values.

- Fixed a bug in `slice2cs --icerpc` where the generated proxy built the response tuple in marshal order while
  declaring it in declaration order. As a result, an operation whose out-parameters were declared in an order
  different from their marshal order could return values in the wrong tuple slots or fail to compile.

### JavaScript Changes

- Assigning an out-of-range or non-integer value to an `InputStream` or `OutputStream` position now throws a
  `RangeError` instead of being silently ignored, matching the buffer-position behavior of the other language
  mappings.

These are the changes since the [Ice 3.8.2] release.

[Ice 3.8.2]: https://github.com/zeroc-ice/ice/blob/3.8/CHANGELOG-3.8.md
